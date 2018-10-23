#include "main.h"
#include "file_system.h"
#include "iap.h"
#include "led.h"
#include "pll.h"
#include "spi.h"
#include "uart.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"

extern char _data_start_lma, _data_start, _data_end, _bss_start, _bss_end;
extern char _flash_start, _flash_end, _ram_start, _ram_end;
extern char _intvecs_size, _text_size, _rodata_size, _data_size, _bss_size, _stack_size, _heap_size;

void main(void) {
   unsigned char buf[64];
   unsigned int error, i, file_addr, flash_addr, sum, file, size, data[2];
   PDRUNCFG &= (~(1<<0 | 1<<1 | 1<<2 | 1<<4 | 1<<7)); //IRC output, IRC, flash, ADC, PLL powered
   SYSAHBCLKCTRL |= (1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<10 | 1<<11 | 1<<14 | 1<<18 | 1<<24); //enable clock for ROM, RAM0_1, FLASHREG, FLASH, I2C0, GPIO, SWM, MRT, SPI0, USART0, IOCON, ADC
   PRESETCTRL |= (1<<0 | 1<<2 | 1<<3 | 1<<6 | 1<<7 | 1<<10 | 1<<11 | 1<<24); //clear SPI0, USART FRG, USART0, I2C0, MRT, GPIO, flash controller, ADC reset

   PLL_Init();
   LED_Init(1);
   SPI0_Init();
   UART_Init();

   UART_Transmit("nemesis bootloader", 1);
   UART_Transmit(__DATE__, 1);
   UART_Transmit(__TIME__, 1);

   fs_mount();
   if(fs_checkdisk()==STATUS_ERROR) {
      fs_format();
      UART_Transmit("fs_checkdisk_error",1);
   }

   if((file=fs_filesearch("nemesis-app.fw")) != STATUS_ERROR) {
      size = fs_filesize(file);
      fs_fileread_datapart(file, 0, 8, (unsigned char*)data);
      if(data[0] == 0x12345678) {
         for(sum=0,file_addr=8; file_addr < size-8; file_addr+=64) {
            fs_fileread_datapart(file, file_addr, 64, buf);
            for(i=0; i<64; i++) {
               sum = (sum>>1) + ((sum&1)<<15);
               sum += buf[i];
               sum &= 0xffff;
            }
         }
         if(sum==data[1]) {
            for(error=0, flash_addr=0x1000,file_addr=8; file_addr < size-8 && !error; file_addr+=64, flash_addr+=64) {
               fs_fileread_datapart(file, file_addr, 64, buf);
               if(iap_erase_page(flash_addr>>6, flash_addr>>6)==IAP_CMD_SUCCESS && iap_copy_ram_to_flash(flash_addr,buf,64)==IAP_CMD_SUCCESS && iap_compare(flash_addr, (unsigned int)buf, 64)==IAP_CMD_SUCCESS) {
                  UART_Transmit(".", 0);
               }
               else {
                  error = 1;
                  UART_Transmit("flash erase/write/compare error", 1);
               }
            }
            if(!error)
               UART_Transmit("\r\nok", 1);
         }
         else {
            UART_Transmit("checksum error", 1);
         }
      }
      else {
         UART_Transmit("firmware file error", 1);
      }
   }
   else {
      UART_Transmit("firmware file not present", 1);
   }

   VTOR = (0x1000<<7);
   _dsb();
   _set_msp(*((unsigned int*)0x1000));
   _dsb();
   ((void(*)(void))(0x1004))();

   while(1);
}

void init(void) {
   char *dst, *src;
   //copy data to ram
   for(src = &_data_start_lma, dst = &_data_start; dst < &_data_end; src++, dst++)
      *dst = *src;
   //zero bss
   for(dst = &_bss_start; dst < &_bss_end; dst++)
      *dst = 0;
}
