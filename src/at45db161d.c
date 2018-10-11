#include "at45db161d.h"
#include "output.h"
#include "spi.h"
#include "utils.h"
#include "lpc824.h"

void static inline AT45DB161D_make_address(unsigned char*,int,int);

void static inline AT45DB161D_make_address(unsigned char *addr,int page,int byte) {
   addr[0] = (page>>(FLASH_PAGE_SIZE==528?6:7)) & (FLASH_PAGE_SIZE==528?0x3f:0x1f);
   addr[1] = ((page&(FLASH_PAGE_SIZE==528?0x3f:0x7f))<<(FLASH_PAGE_SIZE==528?2:1)) | ((byte>>8) & (FLASH_PAGE_SIZE==528?0x3:0x1));
   addr[2] = byte & 0xff;
}

void AT45DB161D_power_down(int mode) { //1 - enter, 0 - resume
   unsigned char out[1];
   out[0] = (mode==0 ? 0xab : 0xb9); //resume from deep power-down : enter deep power-down
   SPI0_Transaction(out,1,1,0,0,0);
}

void AT45DB161D_sector_protection(int mode) { //0 - disable, 1 - enable
   unsigned char out[4] = {0x3d,0x2a,0x7f}; //disable sector protection : enable sector protection
   out[3] = (mode==0 ? 0x9a : 0xa9);
   SPI0_Transaction(out,4,1,0,0,0);
}

void AT45DB161D_erase_page(int page) {
   unsigned char out[4];
   out[0] = 0x81; //page erase
   AT45DB161D_make_address(out+1,page,0);
   SPI0_Transaction(out,4,1,0,0,0);
}

void AT45DB161D_erase_block(int block) {
   unsigned char out[4];
   out[0] = 0x50; //block erase
   AT45DB161D_make_address(out+1,block<<3,0);
   SPI0_Transaction(out,4,1,0,0,0);
}

void AT45DB161D_erase_chip(void) {
   unsigned char out[4] = {0xc7,0x94,0x80,0x9a}; //chip erase
   SPI0_Transaction(out,4,1,0,0,0);
}

void AT45DB161D_read_sector_protection_register(unsigned char *data) { //data must be 16B array
   unsigned char out[1] = {0x32}; //read sector protection register
   SPI0_Transaction(out,1,1,data,16,5);
}

void AT45DB161D_read_sector_lockdown_register(unsigned char *data) { //data must be 16B array
   unsigned char out[1] = {0x35}; //read sector lockdown register
   SPI0_Transaction(out,1,1,data,16,5);
}

void AT45DB161D_read_security_register(unsigned char *data) { //data must be 128B array
   unsigned char out[1] = {0x77}; //read security register
   SPI0_Transaction(out,1,1,data,128,5);
}

union AT45DB161D_status AT45DB161D_read_status(void) {
   unsigned char out[1] = {0xd7};
   union AT45DB161D_status status;
   SPI0_Transaction(out,1,1,(unsigned char*)&status.data,1,2);
   return status;
}

union AT45DB161D_info AT45DB161D_read_info(void) {
   unsigned char out[1] = {0x9f};
   union AT45DB161D_info info;
   SPI0_Transaction(out,1,1,info.data,4,2);
   return info;
}

void AT45DB161D_read_buffer(int buffer,unsigned int starting_byte,unsigned char *buf,int l) {
   unsigned char out[4];
   out[0] = (buffer==1 ? 0xd1 : 0xd3); //buffer read (low frequency mode)
   AT45DB161D_make_address(out+1,0,starting_byte);
   SPI0_Transaction(out,4,1,buf,l,5);
}

void AT45DB161D_write_buffer(int buffer,unsigned int starting_byte,unsigned char *buf,int l) {
   unsigned char out[4];
   out[0] = (buffer==1 ? 0x84 : 0x87); //buffer write
   AT45DB161D_make_address(out+1,0,starting_byte);
   SPI0_Transaction(out,4,1,buf,l,-5);
}

void AT45DB161D_write_buffer_from_page(int buffer,int page) {
   unsigned char out[4];
   out[0] = (buffer==1 ? 0x53 : 0x55);
   AT45DB161D_make_address(out+1,page,0);
   SPI0_Transaction(out,4,1,0,0,0);
}
void AT45DB161D_write_page_from_buffer(int page,int buffer) {
   unsigned char out[4];
   out[0] = (buffer==1 ? 0x83 : 0x86); //buffer to main memory page program with built-in erase
   AT45DB161D_make_address(out+1,page,0);
   SPI0_Transaction(out,4,1,0,0,0);
}

void AT45DB161D_rewrite_page(int page) {
   unsigned char out[4];
   out[0] = 0x59; //auto page rewrite using buffer 2
   AT45DB161D_make_address(out+1,page,0);
   SPI0_Transaction(out,4,1,0,0,0);
}

void AT45DB161D_read(unsigned int addr,unsigned char *buf,int l) {
   unsigned char out[4];
   out[0] = 0x03; //continuous array read (low frequency mode)
   AT45DB161D_make_address(out+1,(addr/FLASH_PAGE_SIZE) & 0xfff,addr%FLASH_PAGE_SIZE);
   SPI0_Transaction(out,4,1,buf,l,5);
}

void AT45DB161D_write(unsigned int addr,unsigned char *buf,int l) {
   int b;
   while(l>0 && addr<FLASH_TOTAL_SIZE) {
      b = MIN2(l,FLASH_PAGE_SIZE-addr%FLASH_PAGE_SIZE);
      while(AT45DB161D_read_status().b.ready==0);
      AT45DB161D_write_buffer_from_page(1, addr/FLASH_PAGE_SIZE);
      while(AT45DB161D_read_status().b.ready==0);
      AT45DB161D_write_buffer(1,addr%FLASH_PAGE_SIZE,buf,b);
      while(AT45DB161D_read_status().b.ready==0);
      AT45DB161D_write_page_from_buffer(addr/FLASH_PAGE_SIZE,1);
      addr += b;
      buf += b;
      l -= b;
   }
}
