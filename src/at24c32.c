#include "at24c32.h"
#include "i2c.h"
#include "utils.h"
#include "lpc824.h"

// at24c32 has 4096 bytes, thus addresses are 0x0 - 0xfff
int AT24C32_read(unsigned int addr,unsigned char *buf,int l) {
   unsigned char address[2],*pdata[2];
   int dir[2],length[2];
   address[0] = (addr>>8)&0xf;
   address[1] = addr&0xff;
   pdata[0] = address;
   pdata[1] = buf;
   dir[0] = 0;
   dir[1] = 1;
   length[0] = 2;
   length[1] = l;
   return I2C_Transaction(0, AT24C32_SLAVE, 2, dir, pdata, length);
}

int AT24C32_write(unsigned int addr,unsigned char *buf,int l) {
   unsigned char address[2], *pdata[2];
   int ok, b,dir[2],length[2];

   for(ok=1; l>0 && addr<AT24C32_SIZE; addr+=b, buf+=b, l-=b) {
      b=MIN2(32-(addr%32),l);
      address[0] = (addr>>8)&0xf;
      address[1] = addr&0xff;
      pdata[0] = address;
      pdata[1] = buf;
      dir[0] = 0;
      dir[1] = 0;
      length[0] = 2;
      length[1] = b;
      while(I2C_Poll(0, AT24C32_SLAVE, 0)==0);
      ok = ok && I2C_Transaction(0, AT24C32_SLAVE, 2, dir, pdata, length);
   }
   return ok;
}
