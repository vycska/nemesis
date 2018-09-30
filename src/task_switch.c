#include "task_switch.h"
#include "fifos.h"
#include "main.h"
#include "os.h"
#include "output.h"
#include "switch.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"

extern volatile long long int millis;
extern volatile struct Switch_Data switch_data;

int smphr_switch;

void Task_Switch(void) {
   char buf[32];

   output("Task_Switch has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   Switch_Init();

   Semaphore_Init(&smphr_switch, 0);

   while(1) {
      Task_Blocking_Wait(&smphr_switch);
      switch(switch_data.active) {
         case 1:
            break;
         case 0:
            mysprintf(buf,"switch %d",switch_data.duration);
            output(buf, eOutputSubsystemSwitch, eOutputLevelDebug, 1);
            switch(switch_data.duration/2000) {
               case 1:
                  Fifo_Command_Parser_Put("led_enabled");
                  break;
               case 2:
                  Fifo_Command_Parser_Put("uart_in_enabled");
                  break;
               case 3:
                  Fifo_Command_Parser_Put("config_save");
                  break;
            }
            RISE = (1<<0);
            SIENR = (1<<0); //enable rising edge interrupt for pin selected in PINTSEL0; this is indirect register which operates on IENR register
            break;
      }
   }
}
