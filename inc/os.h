#ifndef __OS_H__
#define __OS_H__

#define NUMTHREADS 7 // number of threads

struct tcb {
   unsigned int *stack_pointer,
                *stack_base;
   int stack_size,
       stack_usage,
       sleep,
       *block;
   char id,
        priority,
        *name;
   struct tcb *next;
};

void OS_Init(int, ...);
void OS_Scheduler(void);

void Semaphore_Init(int *, int);

void Task_Suspend(void);
void Task_Sleep(int);
void Task_Unsleep(struct tcb*);

void Task_Spinning_Wait(int *);
void Task_Spinning_Signal(int *);
void Task_Suspend_Wait(int *);
void Task_Blocking_Wait(int *);
void Task_Blocking_Signal(int *);

void SystemReset(void);

#endif
