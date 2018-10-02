#ifndef __I2C_H__
#define __I2C_H__

void I2C0_Init(void);
int I2C_Transaction(int,int,int,int*,unsigned char**,int*);

#endif
