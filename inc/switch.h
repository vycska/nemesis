#ifndef __SWITCH_H__
#define __SWITCH_H__

struct Switch_Data {
   int active,delay,duration;
};

void Switch_Init(void);
int Switch_Pressed(void);

#endif
