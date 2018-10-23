.syntax unified
.cpu cortex-m0plus
.thumb

.global Reset_Handler

.weak NMI_Handler,HardFault_Handler,SVC_Handler,PendSV_Handler,SysTick_Handler

.section .intvecs, "a"
.word _stack_end
.word Reset_Handler
.word NMI_Handler
.word HardFault_Handler		@ irq -13, exception 3, offset 0x0c
.word 0
.word 0
.word 0
.word _checksum
.word 0
.word 0
.word 0
.word SVC_Handler
.word 0
.word 0
.word PendSV_Handler
.word SysTick_Handler
@ vendor specific interrupts -- none in bootloader

.text
.thumb_func
.type Reset_Handler, %function
Reset_Handler:
bl init
bl main
b .

.thumb_func
NMI_Handler:
.thumb_func
HardFault_Handler:
.thumb_func
SVC_Handler:
.thumb_func
PendSV_Handler:
.thumb_func
SysTick_Handler:
b .

.end
