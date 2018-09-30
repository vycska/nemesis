#ifndef __LED_H__
#define __LED_H__

struct LED_Data {
   int enabled,counter,dc,period;
};

void LED_Init(void);
void LED_On(void);
void LED_Off(void);
void LED_Toggle(void);

#endif
