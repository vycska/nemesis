#include "bme280.h"
#include "fifos.h"
#include "i2c.h"
#include "os.h"
#include "utils.h"
#include "lpc824.h"

struct BME280_Data bme280_data;

int BME280_RegisterRead(unsigned int r, unsigned char *d, int l) {
   unsigned char reg[1];
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = BME280_SLAVE;
   i2c_data.direction = 2;        //write then read
   reg[0] = r;
   i2c_data.buffer[0] = reg;
   i2c_data.length[0] = 1;
   i2c_data.buffer[1] = d;
   i2c_data.length[1] = l;
   ok = I2C_Transaction(0,&i2c_data);
   return ok;
}

int BME280_RegisterWrite(unsigned int r, unsigned char *d, int l) {
   unsigned char reg[1];
   int ok;
   struct I2C_Data i2c_data;

   i2c_data.slave = BME280_SLAVE;
   i2c_data.direction = 0;        //write
   reg[0] = r;
   i2c_data.buffer[0] = reg;
   i2c_data.length[0] = 1;
   i2c_data.buffer[1] = d;
   i2c_data.length[1] = l;
   ok = I2C_Transaction(0,&i2c_data);
   return ok;
}

int BME280_Init(void) {
   unsigned char data[2] = {0};
   int result = 1;

   //read compensation parameters
   result = result && BME280_RegisterRead(0x88, data, 2);
   bme280_data.dig_T1 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x8a, data, 2);
   bme280_data.dig_T2 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x8c, data, 2);
   bme280_data.dig_T3 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x8e, data, 2);
   bme280_data.dig_P1 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x90, data, 2);
   bme280_data.dig_P2 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x92, data, 2);
   bme280_data.dig_P3 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x94, data, 2);
   bme280_data.dig_P4 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x96, data, 2);
   bme280_data.dig_P5 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x98, data, 2);
   bme280_data.dig_P6 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x9a, data, 2);
   bme280_data.dig_P7 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x9c, data, 2);
   bme280_data.dig_P8 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0x9e, data, 2);
   bme280_data.dig_P9 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0xa1, data, 1);
   bme280_data.dig_H1 = data[0];

   result = result && BME280_RegisterRead(0xe1, data, 2);
   bme280_data.dig_H2 = (data[1] << 8) | data[0];

   result = result && BME280_RegisterRead(0xe3, data, 1);
   bme280_data.dig_H3 = data[0];

   result = result && BME280_RegisterRead(0xe4, data, 2);
   bme280_data.dig_H4 = (data[0] << 4) | (data[1] & 0xf);

   result = result && BME280_RegisterRead(0xe5, data, 2);
   bme280_data.dig_H5 = (data[1] << 4) | ((data[0] >> 4) & 0xf);

   result = result && BME280_RegisterRead(0xe7, data, 1);
   bme280_data.dig_H6 = data[0];

   //set oversampling rates
   bme280_data.osrs_h = bme280_data.osrs_p = bme280_data.osrs_t = 5; // oversampling 2^(osrs-1), e.g. 2^(5-1)=16

   data[0] = bme280_data.osrs_h;
   result = result && BME280_RegisterWrite(0xf2, data, 1);

   data[0] = (bme280_data.osrs_t << 5) | (bme280_data.osrs_p << 2);
   result = result && BME280_RegisterWrite(0xf4, data, 1);

   //set config
   data[0] = (0<<2); //filter off
   result = result && BME280_RegisterWrite(0xf5, data, 1);

   return result;
}

int BME280_GetID(unsigned char *id) {
   return BME280_RegisterRead(0xd0, id, 1);
}

int BME280_StartForcedMeasurement(void) {
   unsigned char data[1];
   int result=1;

   data[0] = bme280_data.osrs_h;
   result = result && BME280_RegisterWrite(0xf2, data, 1);
   data[0] = (bme280_data.osrs_t << 5) | (bme280_data.osrs_p << 2) | (1<<0);
   result = result && BME280_RegisterWrite(0xf4, data, 1);
   return result;
}

int BME280_ReadData(void) {
   unsigned char data[8] = {0};
   int result;

   result = BME280_RegisterRead(0xf7, data, 8);
   bme280_data.up = (data[0]<<12) | (data[1]<<4) | (data[2]>>4);
   bme280_data.ut = (data[3]<<12) | (data[4]<<4) | (data[5]>>4);
   bme280_data.uh = (data[6]<<8) | data[7];
   bme280_data.ct = BME280_compensate_T_int32(bme280_data.ut);
   bme280_data.cp = BME280_compensate_P_int64(bme280_data.up);
   bme280_data.ch = BME280_compensate_H_int32(bme280_data.uh);

   return result;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of 5123 equals 51.23 DegC.
int BME280_compensate_T_int32(int adc_T) {
   int var1, var2, T;

   var1 = ((((adc_T >> 3) - ((int)bme280_data.dig_T1 << 1))) * ((int)bme280_data.dig_T2)) >> 11;
   var2 = (((((adc_T >> 4) - ((int)bme280_data.dig_T1)) * ((adc_T >> 4) - ((int)bme280_data.dig_T1))) >> 12) * ((int)bme280_data.dig_T3)) >> 14;
   bme280_data.t_fine = var1 + var2;
   T = (bme280_data.t_fine * 5 + 128) >> 8;
   return T;
}

// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// Output value of 24674867 represents 24674867/256 = 96386.2 Pa = 963.862 hPa
unsigned int BME280_compensate_P_int64(int adc_P) {
   long long int var1, var2, p;

   var1 = ((long long int)bme280_data.t_fine) - 128000;
   var2 = var1 * var1 * (long long int)bme280_data.dig_P6;
   var2 = var2 + ((var1 * (long long int)bme280_data.dig_P5) << 17);
   var2 = var2 + (((long long int)bme280_data.dig_P4) << 35);
   var1 = ((var1 * var1 * (long long int)bme280_data.dig_P3) >> 8) + ((var1 * (long long int)bme280_data.dig_P2) << 12);
   var1 = (((((long long int)1) << 47) + var1)) * ((long long int)bme280_data.dig_P1) >> 33;

   if(var1 == 0) {
      return 0;                 // avoid exception caused by division by zero
   }

   p = 1048576 - adc_P;
   p = (((p << 31) - var2) * 3125) / var1;
   var1 = (((long long int)bme280_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
   var2 = (((long long int)bme280_data.dig_P8) * p) >> 19;
   p = ((p + var1 + var2) >> 8) + (((long long int)bme280_data.dig_P7) << 4);
   return (unsigned int)p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of 47445 represents 47445/1024 = 46.333 %RH
unsigned int BME280_compensate_H_int32(int adc_H) {
   int v_x1_u32r;

   v_x1_u32r = (bme280_data.t_fine - ((int)76800));
   v_x1_u32r =
      (((((adc_H << 14) - (((int)bme280_data.dig_H4) << 20) - (((int)bme280_data.dig_H5) * v_x1_u32r)) + ((int)16384)) >> 15) * (((((((v_x1_u32r * ((int)bme280_data.dig_H6)) >> 10) * (((v_x1_u32r * ((int)bme280_data.dig_H3)) >> 11) + ((int)32768))) >> 10) +
               ((int)2097152)) * ((int)bme280_data.dig_H2) + 8192) >> 14));
   v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int)bme280_data.dig_H1)) >> 4));
   v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
   v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
   return (unsigned int)(v_x1_u32r >> 12);
}
