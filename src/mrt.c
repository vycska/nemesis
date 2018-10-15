#include "mrt.h"
#include "main.h"
#include "switch.h"
#include "lpc824.h"

void MRT_IRQHandler(void);

extern unsigned int gInterruptCause;
extern volatile struct Switch_Data switch_data;

void MRT_Init(void) {
   IPR2 = (IPR2&(~(3<<22))) | (1<<22); //priority 1
   ISER0 = (1<<10); //interrupt enable
}

void MRT0_Delay(int us) {
   CTRL0 = (0<<0 | 1<<1); //interrupt disable, one-shot interrupt mode
   INTVAL0 = (us*CLOCK) | (1u<<31);
   while(STAT0&(1<<1)); //wait while timer is running
}

void MRT1_Start(int ms) {
   CTRL1 = (1<<0 | 0<<1); //enable interrupt, repeat interrupt mode
   INTVAL1 = (ms*CLOCK*1000) | (1u<<31);
}

void MRT1_Stop(void) {
   CTRL1 = 0;
   INTVAL1 = (1u<<31);
}

void MRT_IRQHandler(void) {
   if(STAT0&(1<<0)) { //TIMER0
      STAT0 |= (1<<0); //clear the interrupt request
   }
   if(STAT1&(1<<0)) { //TIMER1 -- process switch
      STAT1 |= (1<<0);
      //process switch
      if(switch_data.active) {
         switch_data.duration += 1;
         if(Switch_Pressed()) {
            switch_data.delay = 0;
         }
         else if(++switch_data.delay==500) {
            switch_data.active = 0;
            switch_data.delay = 0;
            switch_data.duration -= 500;
            MRT1_Stop();
            RISE = (1<<0);
            SIENR = (1<<0);
            gInterruptCause |= (1<<3);
         }
      }
   }
   if(STAT2&(1<<0)) { //TIMER2
      STAT2 |= (1<<0);
   }
   if(STAT3&(1<<0)) { //TIMER3
      STAT3 |= (1<<0);
   }
}
