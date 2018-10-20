#include "output.h"
#include "uart.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <string.h>

struct Output_Data output_data;

void output(char *buf, enum eOutputSubsystem subsystem, enum eOutputLevel level) {
   if(level & output_data.mask[subsystem]) {
      UART_Transmit(buf,strlen(buf),1);
   }
}
