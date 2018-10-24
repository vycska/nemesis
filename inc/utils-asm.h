#ifndef __UTILS_ASM_H__
#define __UTILS_ASM_H__

#include <stdint.h>

void _disable_irq(void);
void _enable_irq(void);
void _dsb(void);
unsigned int _get_msp(void);
void _set_msp(unsigned int);

#endif
