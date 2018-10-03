#ifndef __I2C_H__
#define __I2C_H__

void I2C0_Init(void);
int I2C_Poll(int,unsigned char,int);
int I2C_Transaction(int,unsigned char,int,int*,unsigned char**,int*);

#endif
