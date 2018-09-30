#ifndef __TASK_BME280_H__
#define __TASK_BME280_H__

struct Task_BME280_Data {
   int ready,units_p,base;
   double h, p, t,
          p_base;
};

void Task_BME280(void);

#endif
