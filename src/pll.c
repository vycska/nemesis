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
