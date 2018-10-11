#ifndef __AT45DB161D_H__
#define __AT45DB161D_H__

#define FLASH_PAGE_SIZE (512)
#define FLASH_TOTAL_SIZE (FLASH_PAGE_SIZE*4096)

union AT45DB161D_status {
   struct {
      unsigned char page512:1;
      unsigned char protect_enabled:1;
      unsigned char density:4;
      unsigned char comp_nomatch:1;
      unsigned char ready:1;
   } b;
   unsigned char data;
};

union AT45DB161D_info {
   struct {
      unsigned char manufacturer_id:8;
      unsigned char density_code:5;
      unsigned char family_code:3;
      unsigned char product_version:5;
      unsigned char mlc_code:3;
      unsigned char byte_count:8;
   } b;
   unsigned char data[4];
};

void AT45DB161D_power_down(int mode);
void AT45DB161D_sector_protection(int mode);
void AT45DB161D_erase_page(int page);
void AT45DB161D_erase_block(int block);
void AT45DB161D_erase_chip(void);
void AT45DB161D_read_sector_protection_register(unsigned char *data);
void AT45DB161D_read_sector_lockdown_register(unsigned char *data);
void AT45DB161D_read_security_register(unsigned char *data);
union AT45DB161D_status AT45DB161D_read_status(void);
union AT45DB161D_info AT45DB161D_read_info(void);
void AT45DB161D_read_buffer(int buffer,unsigned int starting_byte,unsigned char *buf,int l);
void AT45DB161D_write_buffer(int buffer,unsigned int starting_byte,unsigned char *buf,int l);
void AT45DB161D_write_buffer_from_page(int buffer,int page);
void AT45DB161D_write_page_from_buffer(int page,int buffer);
void AT45DB161D_rewrite_page(int page);
void AT45DB161D_read(unsigned int addr,unsigned char *buf,int l);
void AT45DB161D_write(unsigned int addr,unsigned char *buf,int l);

#endif



