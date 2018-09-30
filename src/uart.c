#include "uart.h"
#include "fifos.h"
#include "main.h"
#include "os.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern int smphr_uart_input;

volatile struct UART_Data uart_data;

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
   if(uart_data.uart_in_enabled)
      ISER0 = (1<<3); //UART0 interrupt enable
   else
      ICER0 = (1<<3); //UART0 interrupt disable
   USART0CFG = (1<<0 | 1<<2 | 0<<4 | 0<<6 | 0<<9 | 0<<11 | 0<<15); //USART0 enable, 8b data length, no parity, 1 stop bit, no flow control, asynchronous mode, no loopback mode
}

void UART_Transmit(char *s) {
   int i,k=strlen(s);
   for(i=0;i<k+2;i++) { //+2 nes gale pridesim \r\n
      while((USART0STAT&(1<<2))==0); //wait until TXRDY
      USART0TXDAT = (i == k ? '\r' : (i == k + 1 ? '\n' : s[i]));
   }
}

void UART0_IRQHandler(void) {
   unsigned char c;
   if(USART0INTSTAT&(1<<0)) { //RXRDY
      c = USART0RXDAT&0xff;
      if(isprint(c)) {
         uart_data.s[uart_data.i] = c;
         uart_data.i = (uart_data.i + 1) & (UART_IN_MAX-1);
      }
      else if(uart_data.i != 0) {
         //send received command to the FIFO buffer
         ICER0 = (1<<3);
         uart_data.s[uart_data.i] = 0;
         uart_data.i = 0;
         Task_Blocking_Signal(&smphr_uart_input);
         Task_Suspend();
      }
   }
}

/*
divided_clock = MAINCLOCK / uartdiv; u_pcl = divided_clock / (1+mult/256); baud_rate = u_pclk / (16 * brgval)
#define MAINCLOCK 60000000
int main(void) {
   int baud_rate, mult, brgval, uartdiv, min, u_pclk, divided_clock;
   while(printf("desirable baud_rate: "),scanf("%d",&baud_rate)==1 && baud_rate!=0)
      for(min=0,uartdiv=1;uartdiv<=255;uartdiv++)
         if(MAINCLOCK%uartdiv==0)
            for(divided_clock = MAINCLOCK/uartdiv,mult=0;mult<=255;mult++)
               if(divided_clock%(256+mult)==0) {
                  u_pclk = divided_clock/(256+mult)*256;
                  if(u_pclk%(16*baud_rate)==0) {
                     brgval = u_pclk/(baud_rate*16);
                     printf("uartdiv: %d, divided_clock: %d, mult: %d, u_pclk: %d, brgval: %d\n",uartdiv,divided_clock,mult,u_pclk,brgval);
                  }
               }
   return 0;
}
*/
