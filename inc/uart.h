#ifndef __UART_H__
#define __UART_H__

#define UART_IN_MAX 132

void UART_Init(void);
void UART_Transmit(char*,int,int);

struct UART_Data {
   char s[UART_IN_MAX];
   int i,
       mode;
};

#endif
