#include "lpc824.h"

void PLL_Init(void) {
   SYSPLLCLKSEL &= (~(3<<0)); //system pll clock source IRC
   SYSPLLCLKUEN &= (~(1<<0)); //in order for the update to take effect, first write a zero
   SYSPLLCLKUEN |= (1<<0); //update system pll clock source
   SYSPLLCTRL = (SYSPLLCTRL&(~((0x1f<<0) | (3<<5)))) | (4<<0 | 1<<5); //configure dividers [fin=12,clock=30,d=2,m=5,p=2,fcco=240,fout=60]
   while((SYSPLLSTAT&(1<<0))==0); //wait for the pll to lock
   SYSAHBCLKDIV = (SYSAHBCLKDIV & (~(0xff<<0))) | (2<<0); //set system clock divider value
   MAINCLKSEL |= (3<<0); //select main clock source PLL output
   MAINCLKUEN &= (~(1<<0)); //in order for the update to take effect, first write a zero
   MAINCLKUEN |= (1<<0); //update clock source
}

/*
int main(void) {
   int m,p2,d,fin,fcco,fout,clock;
   while(printf("fin: "),scanf("%d",&fin),printf("clock: "),scanf("%d",&clock),fin!=0 && clock!=0) {
      for(d=1;(fout=d*clock)<100;d++) {
         if(fout%fin==0) {
            m = fout/fin;
            if(m>=1 && m<=32)
               for(p2=1;p2<=8;p2*=2) {
                  fcco = fout * p2;
                  if(fcco>=156 && fcco<=320)
                     printf("d: %d, m: %d, p: %d, fin: %d, fcco: %d, fout: %d, clock: %d\n",d,m,p2/2,fin,fcco,fout,clock);
               }
         }
      }
   }
   return 0;
}
*/
