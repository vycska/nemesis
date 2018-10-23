#ifndef __UTILS_ASM_H__
#define __UTILS_ASM_H__

#include <stdint.h>

void DisableInterrupts(void);
void EnableInterrupts(void);
uint32_t StartCritical(void);
void EndCritical(int);
void WaitForInterrupt(void);
uint32_t GetPSR(void);
void _dsb(void);
unsigned int _get_msp(void);
void _set_msp(unsigned int);

#endif
