#include "mrt.h"
#include "led.h"
#include "main.h"
#include "os.h"
#include "switch.h"
#include "task_switch.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"

extern int smphr_switch;
extern volatile long long int millis;
extern struct tcb *RunPt;
extern struct LED_Data led_data;
extern volatile struct Switch_Data switch_data;

void MRT0_Init(void) {
   IPR2 = (IPR2&(~(3<<22))) | (0<<22); //priority 0 (highest)
   ISER0 = (1<<10); //interrupt enable
   CTRL0 = (1<<0 | 0<<1); //enable interrupt, repeat interrupt mode
   INTVAL0 = (1*CLOCK*1000) | 1u<<31; //load value for 1 ms, force load
}

void MRT1_Delay(int ns) {
   CTRL1 = (0<<0 | 1<<1); //interrupt disable, one-shot interrupt mode
   INTVAL1 = (MAX2(ns*CLOCK/1000,1)) | (1u<<31);
   while(STAT1&(1<<1)); //wait while timer is running
}

void MRT2_Init(int ms) { //paleidzia ADC matavimus
   CTRL2 = (1<<0 | 0<<1); //enable interrupt, repeat interrupt mode
   INTVAL2 = (ms*CLOCK*1000) | (1u<<31);
}

void MRT3_Delay(int us) {
   CTRL3 = (0<<0 | 1<<1); //interrupt disable, one-shot interrupt mode
   INTVAL3 = (us*CLOCK) | (1u<<31);
   while(STAT3&(1<<1)); //wait while timer is running
}
void MRT_IRQHandler(void) {
   if(STAT0&(1<<0)) { //TIMER0 -- process sleeping threads
      unsigned int sp;
      struct tcb *cursor;
      STAT0 |= (1<<0); //clear the interrupt request
      millis += 1; //increment milliseconds timer
      //process sleeping threads
      for(cursor=RunPt->next;cursor!=RunPt;cursor=cursor->next)
         if(cursor->sleep > 0)
            cursor->sleep -= 1;
      //process led blinking
      if(led_data.enabled) {
         led_data.counter += 1;
         if(led_data.counter == led_data.dc*led_data.period/100)
            LED_Off();
         else if(led_data.counter == led_data.period) {
            led_data.counter = 0;
            LED_On();
         }
      }
      //process switch
      if(switch_data.active) {
         if(Switch_Pressed()) {
            switch_data.delay = 0;
         }
         else if(++switch_data.delay==500) {
            switch_data.active = 0;
            switch_data.delay = 0;
            switch_data.duration = millis-switch_data.start-500;
            Task_Blocking_Signal(&smphr_switch);
         }
      }
      //process stack usage
      sp = _sp();
      if((unsigned int)RunPt->stack_base - sp > RunPt->stack_usage)
         RunPt->stack_usage = (unsigned int)RunPt->stack_base - sp;
   }
   if(STAT1&(1<<0)) { //TIMER1
      STAT1 |= (1<<0);
   }
   if(STAT2&(1<<0)) { //TIMER2
      STAT2 |= (1<<0);
      ADCSEQA_CTRL |= (1<<26); //start conversion on adc sequence A
   }
   if(STAT3&(1<<0)) { //TIMER3
      STAT3 |= (1<<0);
   }
}
