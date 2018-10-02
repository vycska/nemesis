#include "config.h"
#include "iap.h"
#include "led.h"
#include "output.h"
#include "task_bme280.h"
#include "uart.h"
#include "lpc824.h"
#include <string.h>

extern volatile struct LED_Data led_data;
extern struct Output_Data output_data;
extern volatile struct UART_Data uart_data;
extern struct Task_BME280_Data task_bme280_data;

struct Config_Data config_data[] = {
   {(char*)&led_data.enabled,          (short)sizeof(led_data.enabled)},
   {(char*)output_data.mask,           (short)sizeof(output_data.mask)},
   {(char*)&uart_data.uart_in_enabled, (short)sizeof(uart_data.uart_in_enabled)},
};

int config_load(void) {
   char *p = (char*)(CONFIG_PAGE*64);
   int i,o,total_size,result;
   for(total_size=0,i=0; i<sizeof(config_data)/sizeof(struct Config_Data); i++)
      total_size += config_data[i].size;
   if(p[0]==CONFIG_VERSION && p[1]==total_size) { //config'as geras
      result = 0;
      for(o=2,i=0; i<sizeof(config_data)/sizeof(struct Config_Data); o+=config_data[i++].size)
         memcpy(config_data[i].address_in_ram, p+o, config_data[i].size);
   }
   else result = 1;
   return result;
}

void config_save(void) {
   char p[64];
   int i,o;
   memset(p,0xff,64);
   for(o=2,i=0; i<sizeof(config_data)/sizeof(struct Config_Data); o+=config_data[i++].size)
      memcpy(p+o, config_data[i].address_in_ram, config_data[i].size);
   p[0] = CONFIG_VERSION;
   p[1] = o-2; //konfigo dydis
   iap_erase_page(CONFIG_PAGE, CONFIG_PAGE);
   iap_copy_ram_to_flash(CONFIG_PAGE*64, (unsigned char*)p, 64);
}
