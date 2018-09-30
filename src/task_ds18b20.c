#include "task_ds18b20.h"
#include "ds18b20.h"
#include "fifos.h"
#include "onewire.h"
#include "os.h"
#include "output.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <string.h>

struct Task_DS18B20_Data task_ds18b20_data = {DS18B20_ERROR_VALUE};

void Task_DS18B20(void) {
   char s[80];
   unsigned char data[9];
   int i,k,l;
   double t;

   output("Task_DS18B20 has started", eOutputSubsystemSystem, eOutputLevelDebug, 0);

   DS18B20_Init();

   k = DS18B20_ReadROM(data);

   if(k == DS18B20_OK) {
      l = mysprintf(s, "one-wire device: ");
      for(i = 0; i < 8; i++)
         l += mysprintf(&s[l], "0x%x%s", (unsigned int)data[i], i == 7 ? "." : " ");
      output(s, eOutputSubsystemDS18B20, eOutputLevelNormal, 1);
   }

   while(1) {
      DS18B20_ConvertTAll();
      Task_Sleep(750); //conversion time
      k = DS18B20_ReadScratchpad(0,data);
      if(k==DS18B20_OK) {
         t = DS18B20_GetTemperature(data);
         task_ds18b20_data.temperature = t*100;
         mysprintf(s, "%f1",(char*)&t);
         output(s, eOutputSubsystemDS18B20, eOutputLevelDebug, 1);
      }
      else {
         task_ds18b20_data.temperature = DS18B20_ERROR_VALUE;
      }
      Task_Sleep(1000);
   }
}
