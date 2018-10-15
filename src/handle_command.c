#include "handle_command.h"
#include "at24c32.h"
#include "at45db161d.h"
#include "bme280.h"
#include "config.h"
#include "ds3231.h"
#include "file_system.h"
#include "handle_measurements.h"
#include "i2c.h"
#include "iap.h"
#include "main.h"
#include "output.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern char _flash_start, _flash_end, _ram_start, _ram_end;
extern struct BME280_Data bme280_data;
extern struct Handle_Measurements_Data handle_measurements_data;
extern struct Output_Data output_data;

void Handle_Command(char *pString) {
   char buf[128];
   int i, j, l;
   unsigned int t,params[12] = {0};

   params_fill(pString, params);
   switch(crc16((unsigned char *)params[1], strlen((char *)params[1]))) {
      case 0x6bd8: //reset
         SystemReset();
         break;
      case 0xd89c: //live_time
         break;
      case 0x426e: //config_save
         config_save();
         break;
      case 0x62bf: //x [value at address]
         if(params_count(params)==2 && !params_integer(2,params)) {
            for(t=0, l=1,i=strlen((char*)params[2])-1; i>=2; l*= 16, i--)
               t += l * (((char*)params[2])[i]>='0' && ((char*)params[2])[i]<='9' ? (((char*)params[2])[i]-'0') : (((char*)params[2])[i]>='a' && ((char*)params[2])[i]<='f' ? (10+((char*)params[2])[i]-'a') : (0)));
            if((t&3)==0 && ((t>=(unsigned int)&_flash_start && t<=(unsigned int)&_flash_end) || (t>=(unsigned int)&_ram_start && t<=(unsigned int)&_ram_end))) {
               mysprintf(buf,"0x%x : 0x%x",t,*((unsigned int*)t));
               output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            }
         }
         break;
      case 0xaded: //om [output mask]
         if(params_count(params)==2) {
            for(i=0; i<(int)eOutputSubsystemLast; i++)
               output_data.mask[i] = params[2];
         }
         else if(params_count(params)==3 && params[2]<eOutputSubsystemLast) {
            output_data.mask[params[2]] = params[3];
         }
         else {
            mysprintf(buf,"ADC %d",(int)eOutputSubsystemADC);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"BME280 %d",(int)eOutputSubsystemBME280);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"DS18B20 %d",(int)eOutputSubsystemDS18B20);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"DS3231 %d",(int)eOutputSubsystemDS3231);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"System %d",(int)eOutputSubsystemSystem);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"Switch %d",(int)eOutputSubsystemSwitch);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"None %d",(int)eOutputLevelNone);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"Debug %d",(int)eOutputLevelDebug);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"Normal %d",(int)eOutputLevelNormal);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            mysprintf(buf,"Important %d",(int)eOutputLevelImportant);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            for(i=0; i<(int)eOutputSubsystemLast; i++) {
               mysprintf(buf, "[%d] %u",i,(unsigned int)output_data.mask[i]);
               output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            }
         }
         break;
      case 0xa53c: //bme [bme280 info]
         mysprintf(buf, "dig_T1: %d", (int)bme280_data.dig_T1);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_T2: %d", (int)bme280_data.dig_T2);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_T3: %d", (int)bme280_data.dig_T3);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P1: %d", (int)bme280_data.dig_P1);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P2: %d", (int)bme280_data.dig_P2);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P3: %d", (int)bme280_data.dig_P3);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P4: %d", (int)bme280_data.dig_P4);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P5: %d", (int)bme280_data.dig_P5);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P6: %d", (int)bme280_data.dig_P6);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P7: %d", (int)bme280_data.dig_P7);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P8: %d", (int)bme280_data.dig_P8);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_P9: %d", (int)bme280_data.dig_P9);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_H1: %d", (int)bme280_data.dig_H1);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_H2: %d", (int)bme280_data.dig_H2);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_H3: %d", (int)bme280_data.dig_H3);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_H4: %d", (int)bme280_data.dig_H4);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_H5: %d", (int)bme280_data.dig_H5);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "dig_H6: %d", (int)bme280_data.dig_H6);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "t_fine: %d", (int)bme280_data.t_fine);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "osrs_h: %d", (int)bme280_data.osrs_h);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "osrs_p: %d", (int)bme280_data.osrs_p);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "osrs_t: %d", (int)bme280_data.osrs_t);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "uh: %d", (int)bme280_data.uh);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "up: %d", (int)bme280_data.up);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "ut: %d", (int)bme280_data.ut);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "ch: %d", (int)bme280_data.ch);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "cp: %d", (int)bme280_data.cp);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);

         mysprintf(buf, "ct: %d", (int)bme280_data.ct);
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         break;
      case 0x7f7e: //iap_info
         t = iap_read_part_id();
         l = mysprintf(&buf[0], "Part id: 0x%x\r\n", t);
         t = iap_read_boot_code_version();
         l += mysprintf(&buf[l], "Boot code version: %d.%d\r\n", (t >> 8) & 0xff, t & 0xff);
         t = (unsigned int)iap_read_uid();
         l += mysprintf(&buf[l], "UID: %u %u %u %u", *((unsigned int *)t + 0), *((unsigned int *)t + 1), *((unsigned int *)t + 2), *((unsigned int *)t + 3));
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         break;
      case 0x57e5: //crc [crc16]
         mysprintf(buf, "0x%x", (unsigned int)crc16((unsigned char *)params[2], strlen((char *)params[2])));
         output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         break;
      case 0x734c: //rtc_read [ds3231 read register]
         if(params_count(params)==2 && params[2]<=0x12) {
            unsigned char value;
            DS3231_ReadRegisters(params[2], &value, 1);
            mysprintf(buf,"[%x] : %x",params[2],(unsigned int)value);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0x68f7: //rtc_write [ds3231 write register]
         if(params_count(params)==3 && params[2]<=0x12) {
            DS3231_WriteRegister(params[2], params[3]);
         }
         break;
      case 0xe1a9: //date
         if(params_count(params)==1) { //get date/time from rtc ds3231
            struct tm dt;
            DS3231_GetDate(&dt);
            mysprintf(buf, "%d-%d-%d [%d] %d:%d:%d",
                  1900+dt.tm_year,
                  1+dt.tm_mon,
                  dt.tm_mday,
                  1+dt.tm_wday,
                  dt.tm_hour,
                  dt.tm_min,
                  dt.tm_sec);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            break;
         }
         else if(params_count(params)==8) { //set date/time [2018 09 30 7 18 2 40]
            struct tm dt;
            dt.tm_year = params[2]-1900;
            dt.tm_mon = params[3]-1;
            dt.tm_mday = params[4];
            dt.tm_wday = params[5]-1;
            dt.tm_hour = params[6];
            dt.tm_min = params[7];
            dt.tm_sec = params[8];
            DS3231_SetDate(&dt);
         }
         break;
      case 0x7691: //set_alarm
         if(params_count(params)>1) {
            switch(params[2]) {
               case 1: //set_alarm 1 32 0 25 61 30
                  if(params_count(params)==7) {
                     DS3231_DisableAlarm(1);
                     DS3231_SetAlarm1(params[3],params[4],params[5],params[6],params[7]);
                  }
                  break;
               case 2: //set_alarm 2 32 0 25 61
                  if(params_count(params)==6) {
                     DS3231_DisableAlarm(2);
                     DS3231_SetAlarm2(params[3],params[4],params[5],params[6]);
                  }
                  break;
            }
         }
         break;
      case 0x107d: //disable_alarm
         if(params_count(params)==2 && (params[2]==1 || params[2]==2)) {
            DS3231_DisableAlarm(params[2]);
         }
         break;
      case 0x67bf: //t [temperature]
         if(params_count(params)==1) { //cia tikrinu siaip, del kintamojo t deklaravimo
            double t;
            t = DS3231_GetTemperature()/100.0;
            mysprintf(buf, "ds3231 t: %f2 C", (char *)&t);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0xa93e: //b [battery]
         if(params_count(params)==1) {
            double v;
            v = handle_measurements_data.adc_battery/4095.0*3.3 * 2;
            mysprintf(buf, "%f2 V",(char*)&v);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0x3ca0: //eprom_read [eeprom at24c32 read]
         if(params_count(params)==3) {
            unsigned char *data = (unsigned char*)0x10000000;
            if(AT24C32_read(params[2], data, params[3])) {
               for(i=0; i<params[3]; i++) {
                  mysprintf(buf, "[%x] : %x",params[2]+i, (unsigned int)data[i]);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
               }
            }
            else {
               output("Error", eOutputSubsystemSystem, eOutputLevelImportant);
            }
         }
         else {
            output("Incorrect syntax", eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0xe5b9: //eprom_write [eeprom at24c32 write]
         if(params_count(params)>3 && params[3]<=4 && params_count(params)==3+params[3]) {
            unsigned char data[4];
            for(i=0; i<params[3]; data[i] = (unsigned char)params[3+1+i],i++);
            if(!AT24C32_write(params[2], data, params[3]))
               output("Error", eOutputSubsystemSystem, eOutputLevelImportant);
         }
         else {
            output("Incorrect syntax", eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0x6ca6: //i2c0_test [i2c0 test slave]
         if(params_count(params)==2) {
            t = I2C_Poll(0, params[2]&0xff, 1);
            mysprintf(buf, "0x%x : %u", params[2]&0xff,t);
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0x7e54: //flash_read [flash at45db161d read]
         if(params_count(params)==3) {
            unsigned char *data = (unsigned char*)0x10000000;
            AT45DB161D_read(params[2],data,params[3]);
            for(i=0; i<params[3]; i++) {
               mysprintf(buf, "[0x%x] : 0x%x",params[2]+i,(unsigned int)data[i]);
               output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
            }
         }
         else output("Incorrect syntax", eOutputSubsystemSystem, eOutputLevelImportant);
         break;
      case 0x62fa: //flash_write [flash at45db161d write]
         if(params_count(params)>3 && params[3]<=4 && params_count(params)==3+params[3]) {
            unsigned char data[4];
            for(i=0; i<params[3]; data[i] = (unsigned char)params[3+1+i],i++);
            AT45DB161D_write(params[2], data, params[3]);
         }
         else output("Incorrect syntax", eOutputSubsystemSystem, eOutputLevelImportant);
         break;
      case 0x9d25: //flash_erase_page [flash at45db161d page erase]
         if(params_count(params)==2 && params[2]<4096) {
            AT45DB161D_erase_page(params[2]);
         }
         break;
      case 0xb434: //flash_erase_chip [flash at45db161d chip erase]
         if(params_count(params)==2 && params[2]==2018)
            AT45DB161D_erase_chip();
         break;
      case 0xaf60: //flash_info
         if(params_count(params)==2) {
            unsigned char *data = (unsigned char*)0x10000000;
            union AT45DB161D_status status;
            union AT45DB161D_info info;
            switch(params[2]) {
               case 1:
                  status = AT45DB161D_read_status();
                  mysprintf(buf, "status: 0x%x", (unsigned int)status.data);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "page512: 0x%x", (unsigned int)status.b.page512);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "protect_enabled: 0x%x", (unsigned int)status.b.protect_enabled);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "density: 0x%x", (unsigned int)status.b.density);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "comp_nomatch: 0x%x", (unsigned int)status.b.comp_nomatch);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "ready: 0x%x", (unsigned int)status.b.ready);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  break;
               case 2:
                  info = AT45DB161D_read_info();
                  mysprintf(buf, "info: 0x%x-0x%x-0x%x-0x%x", (unsigned int)info.data[0], (unsigned int)info.data[1], (unsigned int)info.data[2], (unsigned int)info.data[3]);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "manufacturer_id: 0x%x", (unsigned int)info.b.manufacturer_id);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "family_code: 0x%x", (unsigned int)info.b.family_code);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "density_code: 0x%x", (unsigned int)info.b.density_code);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "mlc_code: 0x%x", (unsigned int)info.b.mlc_code);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "product_version: 0x%x", (unsigned int)info.b.product_version);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  mysprintf(buf, "byte_count: 0x%x", (unsigned int)info.b.byte_count);
                  output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  break;
               case 3:
                  AT45DB161D_read_sector_protection_register(data);
                  for(i=0; i<16; i++) {
                     mysprintf(buf, "[%d] : 0x%x",i,(unsigned int)data[i]);
                     output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  }
                  break;
               case 4:
                  AT45DB161D_read_sector_lockdown_register(data);
                  for(i=0; i<16; i++) {
                     mysprintf(buf, "[%d] : 0x%x",i,(unsigned int)data[i]);
                     output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
                  }
                  break;
               case 5:
                  AT45DB161D_read_security_register(data);
                  for(i=0;i<16;i++) {
                     for(l=j=0;j<8;j++)
                        l+=mysprintf(buf+l,"0x%x ",data[i*8+j]);
                     output(buf, eOutputSubsystemSystem,eOutputLevelImportant);
                  }
                  break;
            }
         }
         break;
      case 0xc595: //fs_info
         mysprintf(buf, "FS_TOTAL_SIZE: %u [%u - %u]", FS_TOTAL_SIZE, FS_START_ADDRESS, FS_END_ADDRESS);
         output(buf, eOutputSubsystemSystem,eOutputLevelImportant);
         mysprintf(buf, "SECTORS: %u, SECTOR_SIZE: %u", SECTORS, SECTOR_SIZE);
         output(buf, eOutputSubsystemSystem,eOutputLevelImportant);
         mysprintf(buf, "sizeof(DirectoryEntry): %u, sizeof(FS): %u",sizeof(struct DirectoryEntry), sizeof(struct FS));
         output(buf, eOutputSubsystemSystem,eOutputLevelImportant);
         mysprintf(buf, "fs_mounted: %u, fs_errors: 0x%x, fs_copy: %u", fs_ismounted(), fs_geterrors(), fs_getcopy());
         output(buf, eOutputSubsystemSystem,eOutputLevelImportant);
         for(i=0; i<DIRECTORY_ENTRIES; i++) {
            l = mysprintf(buf, "[%d]: ",i);
            if(!fs_direntryempty(i))
               l += mysprintf(buf+l, "%s %d %d %d %d", fs_filename(i), fs_filesize(i), fs_filedatasize(i),fs_filerecordsize(i), fs_filerecordsize(i));
            output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
         }
         break;
      case 0x3da8: //fs_flush
         fs_flush();
         break;
      default:
         output("Unknown command", eOutputSubsystemSystem, eOutputLevelImportant);
   }
}

void params_fill(char *s, unsigned int *params) {
   char *p,                     //pointer
     l,                         //length
     d,                         //all digits
     k;                         //# params
   int i;                       //iterator

   for(p = s, d = 1, k = 0, l = strlen(s), i = 0; i <= l; i++) {
      if(s[i] == ' ' || i == l) {
         s[i] = 0;
         params[k + 1] = d ? (params[0] |= (1 << (16 + k)), atoi(p)) : (unsigned int)p;
         k += 1;
         d = 1;
         p = &s[i + 1];
      }
      else {
         d = d && isdigit(s[i]);
      }
   }
   params[0] |= (k & 0xff);
}

int params_count(unsigned int *params) { //kiek parametru uzpildyta po params_fill ivykdymo
   return params[0] & 0xff;
}

int params_integer(char k, unsigned int *params) { //ar paremetras #k yra integer'is, jei ne -- jis yra pointeris i stringa
   return ((params[0] >> 16) & (1 << k)) != 0;
}
