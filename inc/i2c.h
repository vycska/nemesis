#ifndef __I2C_H__
#define __I2C_H__

struct I2C_Data {
   unsigned char *buffer[2],
                 slave,
                 direction;
   int length[2];
};

void I2C0_Init(void);
int I2C_Transaction(int,struct I2C_Data*);
int I2C_Transaction2(int,int,int,int*,unsigned char**,int*);

#endif
