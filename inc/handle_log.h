#ifndef __HANDLE_LOG_H__
#define __HANDLE_LOG_H__

#define LOG_FILE_NAME "log.dat"
#define COUNT_RECORDS 32
#define COUNT_FLUSH 2

struct Log_Config {
   int log_file,
       count_records,
       count_flush;
};

struct Log_Record { //sizeof=16
   unsigned int time;
   short        ds18b20_temperature,
                bme280_temperature,
                ds3231_temperature,
                bme280_humidity,
                bme280_pressure,
                adc_battery;
} __attribute__((packed));

void Handle_Log(void);

#endif
