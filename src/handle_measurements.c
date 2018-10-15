#include "handle_measurements.h"
#include "adc.h"
#include "ds18b20.h"
#include "ds3231.h"
#include "bme280.h"
#include "mrt.h"
#include "output.h"
#include "utils.h"
#include "lpc824.h"
#include <time.h>

extern struct BME280_Data bme280_data;

struct Handle_Measurements_Data handle_measurements_data;

void Handle_Measurements(void) {
   char buf[32];
   unsigned char data[9];
   struct tm dt;

   //adc
   ADCSEQA_CTRL |= (1<<26); //start conversion on adc sequence A
   while((ADCDAT2&(1u<<31))==0);
   handle_measurements_data.adc_battery = (ADCDAT2>>4) & 0xfff;
   mysprintf(buf, "adc bat: %d",handle_measurements_data.adc_battery);
   output(buf,eOutputSubsystemADC,eOutputLevelDebug);

   //bme280
   BME280_StartForcedMeasurement();
   MRT0_Delay((1.25 + 2.3 * power(2, bme280_data.osrs_t-1) + 2.3 * power(2, bme280_data.osrs_p-1) + 0.575 + 2.3 * power(2, bme280_data.osrs_h-1) + 0.575)*1000);
   if(BME280_ReadData()==1) {
      handle_measurements_data.bme280_humidity = bme280_data.ch/1024.0;
      handle_measurements_data.bme280_pressure = bme280_data.cp/256.0 * 0.0075006; //1 atm [standard atmosphere] = 760 torr = 101325 Pa = 1.01325 bar; 1 mmHg = 133.322387415 Pa --> 1 Pa = 0.0075006 mmHg
      handle_measurements_data.bme280_temperature = bme280_data.ct/100.0;

      mysprintf(buf, "bme280 h: %f2 %%", (char *)&handle_measurements_data.bme280_humidity);
      output(buf, eOutputSubsystemBME280, eOutputLevelDebug);
      mysprintf(buf, "bme280 p: %f2 mmHg", (char *)&handle_measurements_data.bme280_pressure);
      output(buf, eOutputSubsystemBME280, eOutputLevelDebug);
      mysprintf(buf, "bme280 t: %f2 C", (char *)&handle_measurements_data.bme280_temperature);
      output(buf, eOutputSubsystemBME280, eOutputLevelDebug);
   }
   else {
      handle_measurements_data.bme280_humidity = handle_measurements_data.bme280_pressure = handle_measurements_data.bme280_temperature = 0xffffffff;
   }

   //ds18b20
   DS18B20_ConvertTAll();
   MRT0_Delay(750*1000); //conversion time
   if(DS18B20_ReadScratchpad(0,data)==DS18B20_OK) {
      handle_measurements_data.ds18b20_temperature = DS18B20_GetTemperature(data);
      mysprintf(buf, "ds18b20 t: %f2 C",(char*)&handle_measurements_data.ds18b20_temperature);
      output(buf, eOutputSubsystemDS18B20, eOutputLevelDebug);
   }
   else {
      handle_measurements_data.ds18b20_temperature = 0xffffffff;
   }

   //rtc DS3231 date and temperature
   DS3231_GetDate(&dt);
   handle_measurements_data.date = mktime(&dt);
   mysprintf(buf, "ds3231 date: %u",handle_measurements_data.date);
   output(buf, eOutputSubsystemDS3231, eOutputLevelDebug);
   handle_measurements_data.ds3231_temperature = DS3231_GetTemperature()/100.0;
   mysprintf(buf, "ds3231 t: %f2 C",(char*)&handle_measurements_data.ds3231_temperature);
   output(buf, eOutputSubsystemDS3231, eOutputLevelDebug);
}
