#ifndef __HANDLE_MEASUREMENTS_H__
#define __HANDLE_MEASUREMENTS_H__

struct Handle_Measurements_Data {
   unsigned int adc_battery,
                date;
   float ds18b20_temperature,
         bme280_humidity,bme280_pressure,bme280_temperature,
         ds3231_temperature;
};

void Handle_Measurements(void);

#endif
