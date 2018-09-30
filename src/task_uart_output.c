#include "task_uart_output.h"
#include "fifos.h"
#include "os.h"
#include "output.h"
#include "uart.h"
#include <string.h>

void Task_Uart_Output(void) {
   char *pString;
   int *smphrFinished = 0;

   output("Task_Uart_Output has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   while(1) {
      Fifo_Uart_Output_Get(&pString, &smphrFinished);
      UART_Transmit(pString);
      if(smphrFinished)
         Task_Blocking_Signal(smphrFinished);
   }
}
