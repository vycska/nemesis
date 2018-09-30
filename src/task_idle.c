#include "task_idle.h"
#include "utils-asm.h"

void Task_Idle(void) {
   while(1) {
      WaitForInterrupt();
   }
}
