#include "ds3231.h"
#include "fifos.h"
#include "i2c.h"
#include "os.h"
#include "utils.h"
#include "lpc824.h"
#include <time.h>

int DS3231_SetDate(struct tm *dt) {
   unsigned char data[8],*pdata[1];
   int dir,length;
   data[0] = 0x0; //pradinis registras nuo kurio bus irasoma
   data[1] = (((dt->tm_sec/10)&0x7)<<4) | ((dt->tm_sec%10)&0xf);
   data[2] = (((dt->tm_min/10)&0x7)<<4) | ((dt->tm_min%10)&0xf);
   data[3] = (0<<6) | (((dt->tm_hour/20)&0x1)<<5) | ((dt->tm_hour<20?((dt->tm_hour/10)&0x1):0)<<4) | ((dt->tm_hour%(dt->tm_hour>=20?20:10))&0xf);
   data[4] = (dt->tm_wday+1)&0x7;
   data[5] = (((dt->tm_mday/10)&0x3)<<4) | ((dt->tm_mday%10)&0xf);
   data[6] = (0<<7) | ((((dt->tm_mon+1)/10)&0x1)<<4) | (((dt->tm_mon+1)%10)&0xf);
   data[7] = ((((dt->tm_year+1900)%100/10)&0xf)<<4) | (((dt->tm_year+1900)%10)&0xf);
   pdata[0] = data;
   dir = 0; //i2c write
   length = 8;
   return I2C_Transaction2(0, DS3231_SLAVE, 1, &dir, pdata, &length);
}

int DS3231_GetDate(struct tm *dt) {
   unsigned char reg[1],data[7],*pdata[2];
   int dir[2],length[2],ok;
   reg[0] = 0x0; //pradinis registras nuo kurio pradesim nuskaityti duomenis
   pdata[0] = reg;
   pdata[1] = data;
   dir[0] = 0; //pirmiausiai write
   dir[1] = 1; //po to read
   length[0] = 1;
   length[1] = 7;
   ok = I2C_Transaction2(0, DS3231_SLAVE, 2, dir, pdata, length);
   dt->tm_year = (2000 + ((data[6]>>4)&0xf)*10 + (data[6]&0xf)) - 1900;
   dt->tm_mon = (((data[5]>>4)&0x1)*10 + (data[5]&0xf)) - 1;
   dt->tm_mday = ((data[4]>>4)&0x3)*10 + (data[4]&0xf);
   dt->tm_wday = (data[3]&0x7) - 1;
   dt->tm_hour = ((data[2]>>5)&0x1)*20 + ((data[2]>>4)&0x1)*10 + (data[2]&0xf);
   dt->tm_min = ((data[1]>>4)&0x7)*10 + (data[1]&0xf);
   dt->tm_sec = ((data[0]>>4)&0x7)*10 + (data[0]&0xf);
   return ok;
}

int DS3231_GetTemperature(void) { //return is temperature x100
   unsigned char reg[1],data[2],*pdata[2];
   int dir[2],length[2];
   reg[0] = 0x11; //address of temperature register [there are two of them -- this is first, the other is 0x12]
   pdata[0] = reg;
   pdata[1] = data;
   dir[0] = 0;
   dir[1] = 1;
   length[0] = 1;
   length[1] = 2;
   I2C_Transaction2(0, DS3231_SLAVE, 2, dir, pdata, length);
   return (((data[0]>>7)&0x1)?(-1):(1)) * ((int)(data[0]&0x7f)*100 + ((data[1]>>6)&0x3)*25);
}
