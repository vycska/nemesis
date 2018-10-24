.syntax unified
.cpu cortex-m0plus
.thumb

.text

.global _disable_irq
.global _enable_irq
.global _dsb
.global _get_msp
.global _set_msp

.thumb_func
_disable_irq:
cpsid i
bx lr

.thumb_func
_enable_irq:
cpsie i
bx lr

.thumb_func
_dsb:
dsb sy
bx lr

.thumb_func
_get_msp:
mrs r0,msp
bx lr

.thumb_func
_set_msp:
msr msp, r0
bx lr

.end
