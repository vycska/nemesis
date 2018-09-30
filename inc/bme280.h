#ifndef __BME280_H__
#define __BME280_H__

#define BME280_SLAVE (0x76)

struct BME280_Data {
   unsigned short dig_T1;
   signed short dig_T2;
   signed short dig_T3;
   unsigned short dig_P1;
   signed short dig_P2;
   signed short dig_P3;
   signed short dig_P4;
   signed short dig_P5;
   signed short dig_P6;
   signed short dig_P7;
   signed short dig_P8;
   signed short dig_P9;
   unsigned char dig_H1;
   signed short dig_H2;
   unsigned char dig_H3;
   signed short dig_H4;
   signed short dig_H5;
   signed char dig_H6;
   int t_fine; // t_fine carries fine temperature as global value
   unsigned char osrs_h, osrs_t, osrs_p;
   int uh, up, ut, ch, cp, ct;
};

int BME280_RegisterRead(unsigned int, unsigned char *, int);
int BME280_RegisterWrite(unsigned int, unsigned char *, int);
int BME280_Init(void);
int BME280_GetID(unsigned char *);
int BME280_StartForcedMeasurement(void);
int BME280_ReadData(void);

int BME280_compensate_T_int32(int adc_T);
unsigned int BME280_compensate_P_int64(int adc_P);
unsigned int BME280_compensate_H_int32(int adc_H);

#endif
