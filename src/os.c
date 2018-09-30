#include "os.h"
#include "main.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

unsigned int __attribute__((section (".stacks"))) stacks[768]; //(576+256+576+576+576+256+256) / 4 = 3072 / 4 = 768
struct tcb tcbs[NUMTHREADS], *RunPt;

void OS_Init(int k, ...) {
   int i, stacks_offset;
   va_list va;

   va_start(va, k);
   for(stacks_offset = i = 0; i < k; i++) {
      tcbs[i].id = i;
      tcbs[i].sleep = 0;                                                 // thread initially not sleeeping
      tcbs[i].block = 0;                                                 // thread initially not blocked
      tcbs[i].name = va_arg(va, char *);
      tcbs[i].priority = va_arg(va, int);
      tcbs[i].stack_size = va_arg(va, int);
      stacks_offset += tcbs[i].stack_size/4;
      tcbs[i].stack_base = &stacks[stacks_offset];
      *(tcbs[i].stack_base-1) = 0x01000000;                              // PSR [thumb bit]
      *(tcbs[i].stack_base-2) = (int)va_arg(va, void (*)(void));         // PC
      *(tcbs[i].stack_base-3) = 0x14141414;                              // LR (R14)
      *(tcbs[i].stack_base-4) = 0x12121212;                              // R12
      *(tcbs[i].stack_base-5) = 0x03030303;                              // R3
      *(tcbs[i].stack_base-6) = 0x02020202;                              // R2
      *(tcbs[i].stack_base-7) = 0x01010101;                              // R1
      *(tcbs[i].stack_base-8) = 0x00000000;                              // R0
      *(tcbs[i].stack_base-9) = 0x11111111;                              // R11
      *(tcbs[i].stack_base-10) = 0x10101010;                             // R10
      *(tcbs[i].stack_base-11) = 0x09090909;                             // R9
      *(tcbs[i].stack_base-12) = 0x08080808;                             // R8
      *(tcbs[i].stack_base-13) = 0x07070707;                             // R7
      *(tcbs[i].stack_base-14) = 0x06060606;                             // R6
      *(tcbs[i].stack_base-15) = 0x05050505;                             // R5
      *(tcbs[i].stack_base-16) = 0x04040404;                             // R4
      tcbs[i].stack_usage = 0;
      tcbs[i].stack_pointer = tcbs[i].stack_base-16;                     // thread stack pointer
      tcbs[i].next = &tcbs[i + 1 != k ? (i + 1) : 0];                    // 0 points to 1, 1 points to 2, and so on
   }
   va_end(va);

   RunPt = &tcbs[0];
}

void OS_Scheduler(void) {
   int i, best_priority;
   struct tcb *cursor, *best_task;

   for(best_task = RunPt, best_priority = 128, cursor = RunPt->next, i = 0; i < NUMTHREADS; cursor = cursor->next, i++)
      if(cursor->priority < best_priority && cursor->block == 0 && cursor->sleep == 0) {
         best_priority = cursor->priority;
         best_task = cursor;
      }
   RunPt = best_task;
}

void Semaphore_Init(int *s, int v) {
   *s = v;
}

void Task_Suspend(void) {
   SYST_CVR = 0;
   ICSR |= (1 << 26);           // SysTick exception set pending
}

void Task_Sleep(int ms) {
   RunPt->sleep = ms;
   Task_Suspend();
}

void Task_Unsleep(struct tcb *task) {
   task->sleep = 0;
}

void Task_Spinning_Wait(int *s) { //spinlock semaphore wait
   DisableInterrupts();
   while(*s <= 0) {
      EnableInterrupts();
      DisableInterrupts();
   }
   *s -= 1;
   EnableInterrupts();
}

void Task_Spinning_Signal(int *s) {
   DisableInterrupts();
   *s += 1;
   EnableInterrupts();
}

void Task_Suspend_Wait(int *s) {
   DisableInterrupts();
   while(*s <= 0) {
      EnableInterrupts();
      Task_Suspend();
      DisableInterrupts();
   }
   *s -= 1;
   EnableInterrupts();
}

void Task_Blocking_Wait(int *s) { //blocking semaphore wait
   DisableInterrupts();
   *s -= 1;
   if(*s < 0) {                 //no pass available, thus block
      RunPt->block = s;
      EnableInterrupts();
      Task_Suspend();
   }
   EnableInterrupts();
}

void Task_Blocking_Signal(int *s) {
   int i;
   struct tcb *cursor;

   DisableInterrupts();
   *s += 1;
   if(*s <= 0) {                //wake up one thread
      for(cursor = RunPt->next, i = 0; i < NUMTHREADS && cursor->block != s; i += 1, cursor = cursor->next);
      if(i < NUMTHREADS)
         cursor->block = 0;
   }
   EnableInterrupts();
}

void SystemReset(void) {
   _DSB();
   AIRCR = (AIRCR & (~(1<<1 | 1<<2 | 0xffffu<<16))) | (1<<2 | 0x5fau<<16);
   _DSB();
   while(1);
}
