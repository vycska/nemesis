#ifndef __UART_H__
#define __UART_H__

#define UART_IN_MAX 64

void UART_Init(void);
void UART_Transmit(char*);

struct UART_Data {
   char s[UART_IN_MAX];
   int i,uart_in_enabled;
};

#endif
