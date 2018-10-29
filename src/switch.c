#include "switch.h"
#include "main.h"
#include "mrt.h"
#include "utils-asm.h"
#include "lpc824.h"

extern volatile unsigned int gInterruptCause;

struct Switch_Data switch_data;

void Switch_Init(void) {
   PINENABLE0 |= (1<<5); //SWDIO disabled on PIO0_2
   PIO0_2 = (1<<3 | 1<<5 | 0<<6 | 0<<10 | 0<<11 | 0<<13); //pull-down resistor enabled, hysteresis enabled, input not inverted, open-drain mode disabled, bypass input filter, IOCONCLKDIV0
   DIR0 &= (~(1<<2)); //direction is input
   PINTSEL0 = (2<<0); //PIO0_3 selected for pin interrupt
   ISEL &= (~(1<<0)); //pin interrupt selected in PINTSEL0 is edge sensitive
   IST = (1<<0); //clear rising and falling edge detection for pin selected in PINTSEL0
   SIENR = (1<<0); //enable rising edge interrupt for pin selected in PINTSEL0; this is indirect register which operates on IENR register
   CIENF = (1<<0); //disable falling edge interrupt for pin selected in PINTSEL0
   IPR6 = (IPR6&(~(3<<6))) | (1<<6); //PININT0 priority 1 (0 = highest, 3 = lowest)
   ISER0 = (1<<24); //enable PININT0
}

int Switch_Pressed(void) {
   return (PIN0>>2)&1;
}

void PININT0_IRQHandler(void) {
   CIENR = (1<<0); //disable rising edge interrupt for pin selected in PINTSEL0
   RISE = (1<<0); //clear rising edge detection
   if(gInterruptCause&(1<<0))
      gInterruptCause &= (~(1<<0));
   else
      gInterruptCause |= (1<<0);
   if(switch_data.active==0) {
      switch_data.active = 1;
      switch_data.delay = 0;
      switch_data.duration = 0;
      MRT1_Start(1);
   }
}
