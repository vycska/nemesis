#ifndef __DS3231_H__
#define __DS3231_H__

#include <time.h>

#define DS3231_SLAVE (0x68)

int DS3231_SetDate(struct tm*);
int DS3231_GetDate(struct tm*);
int DS3231_GetTemperature(void);

#endif

