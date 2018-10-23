#include "uart.h"
#include "main.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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
   //IPR0 = (IPR0&(~(3u<<30))) | (2u<<30); //UART0 interrupt priority 2 (0 = highest, 3 = lowest)
   //ISER0 = (1<<3); //UART0 interrupt enable
   USART0CFG = (1<<0 | 1<<2 | 0<<4 | 0<<6 | 0<<9 | 0<<11 | 0<<15); //USART0 enable, 8b data length, no parity, 1 stop bit, no flow control, asynchronous mode, no loopback mode
}

void UART_Transmit(char *s,int flag_addcrlf) {
   int i, k;
   for(k=strlen(s)+(flag_addcrlf?2:0),i=0;i<k;i++) {
      while((USART0STAT&(1<<2))==0); //wait until TXRDY
      USART0TXDAT = (i == k ? '\r' : (i == k + 1 ? '\n' : s[i]));
   }
}
