#ifndef __TASK_DS18B20_H__
#define __TASK_DS18B20_H__

#define DS18B20_ERROR_VALUE (-10000)

struct Task_DS18B20_Data {
   int temperature;
};

void Task_DS18B20(void);

#endif
