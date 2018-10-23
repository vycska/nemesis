.syntax unified
.cpu cortex-m3
.thumb

.text

.global DisableInterrupts
.global EnableInterrupts
.global StartCritical
.global EndCritical
.global WaitForInterrupt
.global GetPSR
.global _dsb
.global _get_imsp
.global _set_msp

.thumb_func
DisableInterrupts:
cpsid i
bx lr

.thumb_func
EnableInterrupts:
cpsie i
bx lr

.thumb_func
StartCritical:
mrs r0,primask
cpsid i
bx lr

.thumb_func
EndCritical:
msr primask,r0
bx lr

.thumb_func
WaitForInterrupt:
wfi
bx lr

.thumb_func
GetPSR:
mrs r0,psr
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
