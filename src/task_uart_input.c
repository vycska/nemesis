#include "task_uart_input.h"
#include "fifos.h"
#include "os.h"
#include "output.h"
#include "uart.h"
#include "lpc824.h"

extern volatile struct UART_Data uart_data;

int smphr_uart_input;

void Task_Uart_Input(void) {
   output("Task_Uart_Input has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);
   Semaphore_Init(&smphr_uart_input, 0);
   while(1) {
      Task_Blocking_Wait(&smphr_uart_input);
      Fifo_Command_Parser_Put((char*)uart_data.s);
      ISER0 = (1<<3);
   }
}
