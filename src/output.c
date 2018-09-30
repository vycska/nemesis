#include "output.h"
#include "fifos.h"
#include "os.h"
#include "uart.h"
#include "utils-asm.h"
#include "lpc824.h"

struct Output_Data output_data;

void output(char *buf, enum eOutputSubsystem subsystem, enum eOutputLevel level, int block) {
   int smphrFinished;
   if(level & output_data.mask[subsystem]) {
      if(block==-1) {
         UART_Transmit(buf);
      }
      else if(block==1) {
         Semaphore_Init(&smphrFinished,0);
         Fifo_Uart_Output_Put(buf, &smphrFinished);
         Task_Blocking_Wait(&smphrFinished);
      }
      else if(block==0) {
         Fifo_Uart_Output_Put(buf, 0);
      }
   }
}
