#include "ds3231.h"
#include "fifos.h"
#include "i2c.h"
#include "os.h"
#include "utils.h"
#include "lpc824.h"
#include <time.h>

int DS3231_ReadRegisters(unsigned char starting_register,unsigned char *data,int k) {
   unsigned char *pdata[2];
   int dir[2],length[2];
   pdata[0] = &starting_register;
   pdata[1] = data;
   dir[0] = 0;
   dir[1] = 1;
   length[0] = 1;
   length[1] = k;
   return I2C_Transaction(0, DS3231_SLAVE, 2, dir, pdata, length);
}

int DS3231_WriteRegister(unsigned char reg,unsigned char value) {
   unsigned char data[2],*pdata[1];
   int dir,length;
   data[0] = reg;
   data[1] = value;
   pdata[0] = data;
   dir = 0;
   length = 2;
   return I2C_Transaction(0, DS3231_SLAVE, 1, &dir, pdata, &length);
}

int DS3231_GetTemperature(void) { //return is temperature x100
   unsigned char data[2] = {0};
   DS3231_ReadRegisters(0x11, data, 2);
   return (((data[0]>>7)&0x1)?(-1):(1)) * ((int)(data[0]&0x7f)*100 + ((data[1]>>6)&0x3)*25);
}

int DS3231_GetDate(struct tm *dt) {
   unsigned char data[7];
   int ok;
   ok = DS3231_ReadRegisters(0x0, data, 7);
   dt->tm_year = (2000 + ((data[6]>>4)&0xf)*10 + (data[6]&0xf)) - 1900;
   dt->tm_mon = (((data[5]>>4)&0x1)*10 + (data[5]&0xf)) - 1;
   dt->tm_mday = ((data[4]>>4)&0x3)*10 + (data[4]&0xf);
   dt->tm_wday = (data[3]&0x7) - 1;
   dt->tm_hour = ((data[2]>>5)&0x1)*20 + ((data[2]>>4)&0x1)*10 + (data[2]&0xf);
   dt->tm_min = ((data[1]>>4)&0x7)*10 + (data[1]&0xf);
   dt->tm_sec = ((data[0]>>4)&0x7)*10 + (data[0]&0xf);
   dt->tm_isdst = -1;
   return ok;
}

int DS3231_SetDate(struct tm *dt) {
   int ok;
   ok = DS3231_WriteRegister(0x0, (((dt->tm_sec/10)&0x7)<<4) | ((dt->tm_sec%10)&0xf));
   ok = ok && DS3231_WriteRegister(0x1, (((dt->tm_min/10)&0x7)<<4) | ((dt->tm_min%10)&0xf));
   ok = ok && DS3231_WriteRegister(0x2, (0<<6) | (((dt->tm_hour/20)&0x1)<<5) | ((dt->tm_hour<20?((dt->tm_hour/10)&0x1):0)<<4) | ((dt->tm_hour%(dt->tm_hour>=20?20:10))&0xf));
   ok = ok && DS3231_WriteRegister(0x3, (dt->tm_wday+1)&0x7);
   ok = ok && DS3231_WriteRegister(0x4, (((dt->tm_mday/10)&0x3)<<4) | ((dt->tm_mday%10)&0xf));
   ok = ok && DS3231_WriteRegister(0x5, (0<<7) | ((((dt->tm_mon+1)/10)&0x1)<<4) | (((dt->tm_mon+1)%10)&0xf));
   ok = ok && DS3231_WriteRegister(0x6, ((((dt->tm_year+1900)%100/10)&0xf)<<4) | (((dt->tm_year+1900)%10)&0xf));
   return ok;
}

int DS3231_SetAlarm1(int day,int dow, int hour, int min, int sec) { //dow = 1, jei day reiskia savaites diena; jei 0 day yra menesio diena
   unsigned char value;
   int ok;
   ok = DS3231_WriteRegister(0x7, (sec<0 || sec>59) ? (1<<7) : ((((sec/10)&0x7)<<4) | ((sec%10)&0xf)));
   ok = ok && DS3231_WriteRegister(0x8, (min<0 || min>59) ? (1<<7) : ((((min/10)&0x7)<<4) | ((min%10)&0xf)));
   ok = ok && DS3231_WriteRegister(0x9, (hour<0 || hour>23) ? (1<<7) : ((0<<6) | (((hour/20)&0x1)<<5) | (((hour<20)?((hour/10)&0x1):0)<<4) | ((hour%(hour>=20?20:10))&0xf)));
   ok = ok && DS3231_WriteRegister(0xa, (day<0 || day>31) ? (1<<7) : (((dow&1)<<6) | (((day/10)&0x3)<<4) | ((day%10)&0xf)));

   ok = ok && DS3231_ReadRegisters(0xe, &value, 1);
   value |= (1<<2) | (1<<0); //set INTCN and A1IE
   ok = ok && DS3231_WriteRegister(0xe, value);
   return ok;
}

int DS3231_SetAlarm2(int day,int dow, int hour, int min) { //dow = 1, jei day reiskia savaites diena; jei 0 day yra menesio diena
   unsigned char value;
   int ok;
   ok = DS3231_WriteRegister(0xb, (min<0 || min>59) ? (1<<7) : ((((min/10)&0x7)<<4) | ((min%10)&0xf)));
   ok = ok && DS3231_WriteRegister(0xc, (hour<0 || hour>23) ? (1<<7) : ((0<<6) | (((hour/20)&0x1)<<5) | (((hour<20)?((hour/10)&0x1):0)<<4) | ((hour%(hour>=20?20:10))&0xf)));
   ok = ok && DS3231_WriteRegister(0xd, (day<0 || day>31) ? (1<<7) : (((dow&1)<<6) | (((day/10)&0x3)<<4) | ((day%10)&0xf)));

   ok = ok && DS3231_ReadRegisters(0xe, &value, 1);
   value |= (1<<2) | (1<<1); //set INTCN and A2IE
   ok = ok && DS3231_WriteRegister(0xe, value);
   return ok;
}

int DS3231_ClearAlarm(int k) {
   unsigned char value;
   int ok;
   ok = DS3231_ReadRegisters(0xf,&value,1);
   value &= ~(1<<(k==1?0:1));
   ok = ok && DS3231_WriteRegister(0xf,value);
   return ok;
}

int DS3231_DisableAlarm(int k) {
   unsigned char value;
   int ok;
   ok = DS3231_ReadRegisters(0xe, &value, 1);
   value &= ~(1<<(k==1?0:1));
   ok = ok && DS3231_WriteRegister(0xe, value);
   ok = ok && DS3231_ClearAlarm(k);
   return ok;
}
