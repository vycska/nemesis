.syntax unified
.cpu cortex-m0plus
.thumb

.text

.global OS_Start
.global SysTick_Handler

.thumb_func
OS_Start:
@load stack pointer from RunPt
ldr r0,=RunPt
ldr r1,[r0]
ldr r2,[r1]
mov sp,r2
@pop r4-r7 and r8-r11 from stack
pop {r4-r7}
pop {r0-r3}
mov r8,r0
mov r9,r1
mov r10,r2
mov r11,r3
@pop r0-r3 and r12 from stack (note r0 saved in psp)
pop {r0-r3}
msr psp,r0
pop {r0}
mov r12,r0
@skip lr
add sp,#4
@pop pc from stack into lr
pop {r0}
mov lr,r0
@skip psr
add sp,#4
@we need to save lr temporarily
push {lr}
@initialize systick and timer for sleeping threads
ldr r0,=10
bl SysTick_Init
bl MRT0_Init
@restore lr from stack
pop {r0}
mov lr,r0
@finally restore r0
mrs r0,psp
@finally, enable interrupts and jump to the task
cpsie i
bx lr

.thumb_func
SysTick_Handler:
@disable interrupts
cpsid i
@store r8-r11 and r4-r7 in the stack
mov r0,r8
mov r1,r9
mov r2,r10
mov r3,r11
push {r0-r3}
push {r4-r7}
@save stack pointer in the tcb
ldr r0,=RunPt
ldr r1,[r0]
mov r2,sp
str r2,[r1]
@save r0, which is RunPt address, and lr (0xfffffff9 probably) into the stack
push {r0,lr}
@run scheduler with argument RunPt in r0
bl OS_Scheduler
@restore r0 and lr
pop {r0,r1}
mov lr,r1
@assign new stack pointer value
ldr r1,[r0]
ldr r2,[r1]
mov sp,r2
@pop r4-r7 and r8-r11 from the new stack
pop {r4-r7}
pop {r0-r3}
mov r8,r0
mov r9,r1
mov r10,r2
mov r11,r3
@enable interrupts and let the stack unroll
cpsie i
bx lr

.end
