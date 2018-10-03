#include "i2c.h"
#include "main.h"
#include "os.h"
#include "utils.h"
#include "lpc824.h"
#include <stdlib.h>

void I2C0_Init(void) {
   PINENABLE0 &= (~(1<<11 | 1<<12)); //I2C0_SDA enabled on pin PIO0_11, I2C0_SCL enabled on pin PIO0_10
   PIO0_11 = (0<<6 | 0<<8 | 0<<11 | 0<<13); //input not inverted, standard/fast mode I2C, bypass input filter, peripheral clock divider 0
   PIO0_10 = (0<<6 | 0<<8 | 0<<11 | 0<<13); //input not inverted, standard/fast mode I2C, bypass input filter, peripheral clock divider 0
   I2C0->CLKDIV = (CLOCK-1); //pclk divider to produce function clock of 1MHz
   I2C0->MSTTIME = (3<<0 | 3<<4); //SCL low time is 3+2, SCL high time is 3+2
   I2C0->TIMEOUT |= (0xfff<<4); //timeout value (will not be used)
   I2C0->INTENCLR = (1<<0 | 1<<4 | 1<<6 | 1<<8 | 1<<11 | 1<<15 | 1<<16 | 1<<17 | 1<<19 | 1<<24 | 1<<25); //no interrupts
   //IPR2 = (IPR2&(~(3<<6))) | (1<<6); //I2C0 interrupt priority 1 (0 = highest, 3 = lowest)
   //ISER0 = (1<<8); //I2C0 interrupt enable
   I2C0->CFG = (1<<0 | 0<<1 | 0<<2 | 0<<3 | 0<<4); //master enable, slave disable, monitor disabled, timeout disabled, monitor clock stretching disabled
}

int I2C_Poll(int block, unsigned char slave, int rw) {
   unsigned char *pdata[1]={0};
   int dir=rw, length=0;
   return I2C_Transaction(block, slave, 1, &dir, pdata, &length);
}

int I2C_Transaction(int block, unsigned char slave, int k, int *dir,unsigned char **pdata, int *length) {
   unsigned char *data;
   int i,j,ok;
   struct I2C_Registers *I2C;

   switch(block) {
      case 0:
         I2C = I2C0;
         break;
      case 1:
         I2C = I2C1;
         break;
      case 2:
         I2C = I2C2;
         break;
      case 3:
         I2C = I2C3;
         break;
      default:
         I2C = I2C0;
   }

   for(ok=1,i=0;i<k && ok;i++) { //k buferiai
      for(data=pdata[i],j=-1;j<length[i] && ok;j++) {
         while((I2C->STAT&(1<<0))==0); //wait for pending status to be set
         if(j==-1 && (i==0 || dir[i]!=dir[i-1])) {
            I2C->MSTDAT = ((slave<<1) | dir[i]); //write the slave address with RW
            I2C->MSTCTL = (1<<1); //start the transmission by setting the MSTSTART
            while((I2C->STAT&(1<<0))==0); //wait for pending status to be set by polling STAT register
            ok = (dir[i]==1)?(((I2C->STAT>>1)&0x7)==1):(((I2C->STAT>>1)&0x7)==2);
         }
         else if(j>=0)
            ok = ((I2C->STAT>>1)&0x7)!=4; //slave did not NACKed transmitted data
         if(ok && j>=0) {
            if(dir[i]==1)
               data[j] = I2C->MSTDAT;
            else //write 8 bits of data to the MSTDAT
               I2C->MSTDAT = data[j];
            I2C->MSTCTL = (1<<0); //continue with the transmission by setting MSTCONT
         }
      }
   }
   while((I2C->STAT&(1<<0))==0); //wait for pending status to be set
   I2C->MSTCTL = (1<<2); //stop the transmission by setting MSTSTOP
   return ok;
}
