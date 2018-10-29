#include "uart.h"
#include "fifos.h"
#include "handle_xmodem.h"
#include "main.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern volatile unsigned int gInterruptCause;
extern struct Handle_Xmodem_Data handle_xmodem_data;

static struct UART_Data uart_data;

//baudrate: 9600, main_clock: 60000000, uartdiv: 250, divided_clock: 240000, mult: 144, u_pclk: 153600, brgval: 1
//baudrate: 38400, main_clock: 60000000, uartdiv: 5, divided_clock: 12000000, mult: 244, u_pclk: 6144000, brgcal: 10
void UART_Init(void) {
   USART0CFG &= (~(1<<0)); //disable usart0
   PINENABLE0 |= (1<<0 | 1<<24); //disable ACMP_I1 on PIO0_0, disable ADC_11 on PIO0_4
   PINASSIGN0 = (PINASSIGN0&(~(0xff<<0 | 0xff<<8))) | (4<<0 | 0<<8); //U0TXD assigned to PIO0_4, U0RXD assigned to PIO0_0
   UARTCLKDIV = 5; //configure the clock divider for the fractional baud rate generator
   UARTFRGDIV = 0xff; //denominator of the fractional divider
   UARTFRGMULT = 244; //numerator of the fractional divider
   USART0BRG = 9; //this value+1 is used to divide clock to determine the baud rate
   USART0OSR= 0xf; //oversample 16
   USART0CTL = (0<<1 | 0<<2 | 0<<6 | 0<<16); //no break, no address detect mode, transmit not disabled, autobaud disabled
   USART0INTENSET = (1<<0); //interrupt when there is a received character
   IPR0 = (IPR0&(~(3u<<30))) | (2u<<30); //UART0 interrupt priority 2 (0 = highest, 3 = lowest)
   ISER0 = (1<<3); //UART0 interrupt enable
   USART0CFG = (1<<0 | 1<<2 | 0<<4 | 0<<6 | 0<<9 | 0<<11 | 0<<15); //USART0 enable, 8b data length, no parity, 1 stop bit, no flow control, asynchronous mode, no loopback mode
}

void UART_Transmit(char *s,int k,int flag_addcrlf) {
   int i;
   for(i=0;i<k+(flag_addcrlf?2:0);i++) { //+2 nes gale pridesim \r\n
      while((USART0STAT&(1<<2))==0); //wait until TXRDY
      USART0TXDAT = (i == k ? '\r' : (i == k + 1 ? '\n' : s[i]));
   }
}

void UART0_IRQHandler(void) {
   unsigned char c;
   if(USART0INTSTAT&(1<<0)) { //RXRDY
      c = USART0RXDAT&0xff;
      switch(uart_data.mode) {
         case eUARTReceivingModeCommands:
            if(isprint(c)) {
               uart_data.s[uart_data.i++] = c;
               if(uart_data.i>=UART_IN_MAX)
                  uart_data.i = 0;
            }
            else if(uart_data.i != 0) {
               uart_data.s[uart_data.i] = 0;
               uart_data.i = 0;
               Fifo_Command_Parser_Put(uart_data.s);
            }
            break;
         case eUARTReceivingModeXMODEMSending:
            Fifo_Xmodem_Sending_Put(c);
            break;
         case eUARTReceivingModeXMODEMReceiving:
            uart_data.s[uart_data.i++] = c;
            if((uart_data.i==1 && (c==CAN || c==EOT)) || uart_data.i==132) {
               if(handle_xmodem_data.receiving_ready==0) {
                  memcpy((char*)handle_xmodem_data.receiving_data, uart_data.s, uart_data.i);
                  handle_xmodem_data.receiving_size = uart_data.i;
                  handle_xmodem_data.receiving_ready = 1;
               }
               else handle_xmodem_data.receiving_lost += 1;
               uart_data.i = 0;
            }
            break;
      }
   }
}

void UART_ReceivingMode_Change(enum UARTReceivingMode m) {
   uart_data.mode = m;
}

void UART_ReceivingData_Reset(void) {
   uart_data.i = 0;
}
