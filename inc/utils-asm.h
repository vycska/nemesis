#ifndef __UTILS_ASM_H__
#define __UTILS_ASM_H__

#include <stdint.h>

void _disable_irq(void);
void _enable_irq(void);
void _dsb(void);
uint32_t _get_msp(void);
void _set_msp(uint32_t);

#endif
