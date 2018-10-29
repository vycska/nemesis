#ifndef __UART_H__
#define __UART_H__

#define UART_IN_MAX 132

enum UARTReceivingMode {
   eUARTReceivingModeCommands          = 0,
   eUARTReceivingModeXMODEMSending     = 1,
   eUARTReceivingModeXMODEMReceiving   = 2
};

struct UART_Data {
   enum UARTReceivingMode mode;
   char s[UART_IN_MAX];
   int i;
};

void UART_Init(void);
void UART_Transmit(char*,int,int);
void UART_ReceivingMode_Change(enum UARTReceivingMode);
void UART_ReceivingData_Reset(void);

#endif
