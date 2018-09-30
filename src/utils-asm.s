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
.global _DSB
.global _sp

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
_DSB:
dsb sy
bx lr

.thumb_func
_sp:
mrs r0,msp
bx lr

.end
