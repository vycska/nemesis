#ifndef __DS3231_H__
#define __DS3231_H__

#include <time.h>

#define DS3231_SLAVE (0x68)

void DS3231_Init(void);
int DS3231_ReadRegisters(unsigned char starting_register,unsigned char *data,int k);
int DS3231_WriteRegister(unsigned char reg,unsigned char value);
int DS3231_GetTemperature(void);
int DS3231_GetDate(struct tm *dt);
int DS3231_SetDate(struct tm *dt);
int DS3231_SetAlarm1(int day,int dow, int hour, int min, int sec);
int DS3231_SetAlarm2(int day,int dow, int hour, int min);
int DS3231_ClearAlarm(int k);
int DS3231_DisableAlarm(int k);

#endif

