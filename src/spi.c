#include "spi.h"
#include "main.h"
#include "utils.h"
#include "lpc824.h"

void SPI0_Init(void) {
   PINENABLE0 |= (1<<4 | 1<<8 | 1<<22 | 1<<23); //SWCLK disabled in PIO0_3, RESETN disabled on PIO0_5, ADC_9 disabled on PIO0_17, ADC_10 disabled on PIO0_13
   PIO0_3 &= (~(0x3<<3 | 1<<5 | 1<<6 | 1<<10 | 0x3<<11 | 0x7<<13));
   PIO0_5 &= (~(0x3<<3 | 1<<5 | 1<<6 | 1<<10 | 0x3<<11 | 0x7<<13));
   PIO0_13 &= (~(0x3<<3 | 1<<5 | 1<<6 | 1<<10 | 0x3<<11 | 0x7<<13));
   PIO0_17 &= (~(0x3<<3 | 1<<5 | 1<<6 | 1<<10 | 0x3<<11 | 0x7<<13));
   PINASSIGN3 = (PINASSIGN3 & (~(0xff<<24))) | (13<<24); //SPI0_SCK_IO on PIO0_13
   PINASSIGN4 = (PINASSIGN4 & (~(0xff<<0 | 0xff<<8 | 0xff<<16))) | (5<<0) | (3<<8) | (17<<16); //SPIO0_MOSI_IO on PIO0_5, SPI0_MISO_IO on PIO0_3, SPIO0_SSEL0_IO on PIO0_17
   SPI0->CFG = (0<<0 | 1<<2 | 0<<3 | 0<<4 | 0<<5 | 0<<7 | 0<<8); //disable SPI, select master mode, MSB first mode, CPHA=0, CPOL=0, disable loopback, SSEL0 is active low
   SPI0->DIV = 1;
   SPI0->DLY = (0<<0 | 0<<4 | 0<<8 | 0<<12); //pre-delay, post-delay, frame-delay, transfer delay
   SPI0->INTENCLR = (1<<0 | 1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5); //disable RXRD, TXRD, RXOV, TXUR, SSA and SSD interrupts
   SPI0->TXCTL = (0xf<<16 | 0<<20 | 0<<21 | 0<<22 | 7<<24); //SSEL0-3 not asserted, SSEL not deasserted, data not EOF, receive not ignored, data length (7+1) bits
   SPI0->STAT = (1<<4 | 1<<5);
   SPI0->CFG = (1<<0 | 1<<2 | 0<<3 | 0<<4 | 0<<5 | 0<<7 | 0<<8); //enable SPI, select master mode, MSB first mode, CPHA=0, CPOL=0, disable loopback, SSEL0 is active low [antra karta taip viska surasau, nes parasyta, kad "reserved bits read value is undefined, only zero should be written"]
}

void SPI0_Transaction(unsigned char *out,int out_length,int out_start,unsigned char *in,int in_length,int in_start) { //jei in_start<0, masyvas "in" yra irasinejamas i slave'a
   int i,total;
   unsigned int r;
   SPI0->TXCTL = (0<<16 | 1<<17 | 1<<18 | 1<<19 | 0<<20 | 0<<21 | 0<<22 | 7<<24); //assert SSEL0
   for(total=MAX2(out_start+out_length-1,ABS(in_start)+in_length-1),i=1;i<=total;i++) {
      if(i==total && total!=1) SPI0->TXCTL = (1<<16 | 1<<17 | 1<<18 | 1<<19 | 1<<20 | 0<<21 | 0<<22 | 7<<24); //set SSEL0 not asserted, EOT is set
      while(((SPI0->STAT>>1)&1) == 0); //wait for TXRDY
      SPI0->TXDAT = (i>=out_start && i<=out_start+out_length-1 ? out[i-out_start] : (in_start<0 && i>=ABS(in_start) && i<=ABS(in_start)+in_length-1 ? in[i-ABS(in_start)]: 0xff));
      while(((SPI0->STAT>>0)&1) == 0); //wait for RXRDY
      r = SPI0->RXDAT;
      if(in_start>0 && i>=in_start && i<=in_start+in_length-1) in[i-in_start] = r&0xff;
      if(total == 1) SPI0->STAT |= (1<<7); //set ENDOFTRANSFER
   }
   SPI0->TXCTL = (0xf<<16 | 0<<20 | 0<<21 | 0<<22 | 7<<24); //all SSEL not asserted, EOT cleared
   SPI0->STAT = (1<<4 | 1<<5); //clear slave select assert and deassert flags
}
