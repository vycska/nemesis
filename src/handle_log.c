#include "handle_log.h"
#include "handle_measurements.h"
#include "file_system.h"

static void gather_data(struct Log_Record*);

extern struct Handle_Measurements_Data handle_measurements_data;

struct Log_Config log_config;
struct Log_Record log_records[COUNT_RECORDS]; //COUNT_RECORDS * sizeof(Log_Record) = 512

void Handle_Log(void) {
   if((log_config.log_file=fs_filesearch(LOG_FILE_NAME)) == STATUS_ERROR) {
      log_config.log_file = fs_filenew(LOG_FILE_NAME, 240, sizeof(struct Log_Record));
      fs_flush();
   }
   gather_data(&log_records[log_config.count_records]);
   log_config.count_records += 1;
   if(log_config.log_file != STATUS_ERROR && log_config.count_records >= COUNT_RECORDS) {
      fs_fileappend(log_config.log_file, (char*)&log_records, COUNT_RECORDS*sizeof(struct Log_Record));
      log_config.count_records = 0;
      log_config.count_flush += 1;
      if(log_config.count_flush >= COUNT_FLUSH) {
         fs_flush();
         log_config.count_flush = 0;
      }
   }
}

static void gather_data(struct Log_Record *d) {
   d->time = handle_measurements_data.date;
   d->ds18b20_temperature = handle_measurements_data.ds18b20_temperature * 100;
   d->bme280_temperature = handle_measurements_data.bme280_temperature * 100;
   d->ds3231_temperature = handle_measurements_data.ds3231_temperature * 100;
   d->bme280_humidity = handle_measurements_data.bme280_humidity * 10;
   d->bme280_pressure = handle_measurements_data.bme280_pressure * 10;
   d->adc_battery = handle_measurements_data.adc_battery;
}
