#include "systick.h"
#include "main.h"
#include "lpc824.h"

void SysTick_Init(int ms) {
   SYST_CSR = (SYST_CSR & (~(1<<0 | 1<<1))) | (1<<2); //counter disabled, interrupt disabled, clock source is the system clock (CPU)
   SHPR3 = (SHPR3 & (~(0x3u<<30))) | (3u<<30); //set the lowest priority [3]
   SYST_RVR = ms * 1000 * CLOCK - 1; //reload value
   SYST_CVR = 0; //writing any value clear counter and the COUNTFLAG bit
   SYST_CSR |= (1<<0 | 1<<1); //counter enable, interrupt enable
}
