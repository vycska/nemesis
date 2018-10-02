#include "task_bme280.h"
#include "bme280.h"
#include "fifos.h"
#include "os.h"
#include "output.h"
#include "utils.h"
#include <string.h>

extern int mutex_i2c0;
extern struct BME280_Data bme280_data;

struct Task_BME280_Data task_bme280_data;

void Task_BME280(void) {
   char s[24];
   int result;
   unsigned char id[1];

   output("Task_BME280 has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   Task_Sleep(2); //max time for first communication

   Task_Blocking_Wait(&mutex_i2c0);
   result = BME280_Init();
   if(result==1)
      output("BME280 init ok", eOutputSubsystemBME280, eOutputLevelDebug, 1);
   else
      output("BME280 init error", eOutputSubsystemBME280, eOutputLevelDebug, 1);

   result = BME280_GetID(id);
   if(result==1) {
      mysprintf(s,"BME280 id: %x",(unsigned int)id[0]);
      output(s, eOutputSubsystemBME280, eOutputLevelDebug, 1);
   }
   else
      output("BME280 id read error", eOutputSubsystemBME280, eOutputLevelDebug, 1);
   Task_Blocking_Signal(&mutex_i2c0);

   while(1) {
   Task_Blocking_Wait(&mutex_i2c0);
      result = BME280_StartForcedMeasurement();
      if(result==1)
         output("BME280 start measurement ok", eOutputSubsystemBME280, eOutputLevelDebug, 1);
      else
         output("BME280 start measurement error", eOutputSubsystemBME280, eOutputLevelDebug, 1);

      Task_Sleep(1.25 + 2.3 * power(2, bme280_data.osrs_t-1) + 2.3 * power(2, bme280_data.osrs_p-1) + 0.575 + 2.3 * power(2, bme280_data.osrs_h-1) + 0.575);
      result = BME280_ReadData();
      Task_Blocking_Signal(&mutex_i2c0);
      if(result==1) {
         task_bme280_data.t = bme280_data.ct/100.0;
         task_bme280_data.h = bme280_data.ch/1024.0;
         task_bme280_data.p = bme280_data.cp/256.0 * 0.0075006; //1 atm [standard atmosphere] = 760 torr = 101325 Pa = 1.01325 bar; 1 mmHg = 133.322387415 Pa --> 1 Pa = 0.0075006 mmHg
         task_bme280_data.ready = 1;

         mysprintf(s, "bme280 t: %f2 C", (char *)&task_bme280_data.t);
         output(s, eOutputSubsystemBME280, eOutputLevelDebug, 1);
         mysprintf(s, "bme280 h: %f2 %%", (char *)&task_bme280_data.h);
         output(s, eOutputSubsystemBME280, eOutputLevelDebug, 1);
         mysprintf(s, "bme280 p: %f2 mmHg", (char *)&task_bme280_data.p);
         output(s, eOutputSubsystemBME280, eOutputLevelDebug, 1);
      }
      else {
         task_bme280_data.ready = 0;
         output("BME280 read error", eOutputSubsystemBME280, eOutputLevelDebug, 1);
      }

      Task_Sleep(2000);
   }
}
