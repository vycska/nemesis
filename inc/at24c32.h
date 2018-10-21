#ifndef __AT24C32_H__
#define __AT24C32_H__

#define AT24C32_SLAVE 0x57
#define AT24C32_SIZE 0x1000

int AT24C32_read(unsigned int address,unsigned char *buf,int l);
int AT24C32_write(unsigned int address,unsigned char *buf,int l);

#endif


