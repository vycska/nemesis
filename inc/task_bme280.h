#ifndef __TASK_BME280_H__
#define __TASK_BME280_H__

struct Task_BME280_Data {
   int ready;
   double h, p, t;
};

void Task_BME280(void);

#endif
