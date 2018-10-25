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

#define FW_FILE_NAME ".fw"
#define FW_FLASH_ADDR 0x1400
#define FW_BLOCK_SIZE 1024 //dydis 1024 sutampa su sektoriaus dydziu, todel as naudoju erase_sector [o ne erase_page]

extern char _data_start_lma, _data_start, _data_end, _bss_start, _bss_end;

void main(void) {
   unsigned char buf[FW_BLOCK_SIZE];
   int i, j, res, error, file, size;
   unsigned int sum, data[3];

   _disable_irq();

   PDRUNCFG &= (~(1<<0 | 1<<1 | 1<<2 | 1<<4 | 1<<7)); //IRC output, IRC, flash, ADC, PLL powered
   SYSAHBCLKCTRL |= (1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<10 | 1<<11 | 1<<14 | 1<<18 | 1<<24); //enable clock for ROM, RAM0_1, FLASHREG, FLASH, I2C0, GPIO, SWM, MRT, SPI0, USART0, IOCON, ADC
   PRESETCTRL |= (1<<0 | 1<<2 | 1<<3 | 1<<6 | 1<<7 | 1<<10 | 1<<11 | 1<<24); //clear SPI0, USART FRG, USART0, I2C0, MRT, GPIO, flash controller, ADC reset

   PLL_Init();
   LED_Init(1);
   SPI0_Init();
   UART_Init();

   UART_Transmit("\r\n\r\nnemesis bootloader v1", 1);
   UART_Transmit(__DATE__, 1);
   UART_Transmit(__TIME__, 1);

   fs_mount();
   if(fs_checkdisk()==STATUS_ERROR)
      fs_format();

   if((file=fs_filesearch_tail(FW_FILE_NAME)) != STATUS_ERROR) {
      UART_Transmit("firmware file found", 1);
      size = fs_filesize(file);
      fs_fileread_datapart(file, 0, 12, (unsigned char*)data);
      if(data[0] == 0x12345678 && data[1]+12<=size && data[1]>=FW_BLOCK_SIZE && data[1]<=0x8000-FW_FLASH_ADDR) {
         UART_Transmit("firmware file ok", 1);
         for(sum=0,i=0; i<(data[1]>>9); i++) {
            fs_fileread_datapart(file, 12+i*FW_BLOCK_SIZE, FW_BLOCK_SIZE, buf);
            for(j=0; j<FW_BLOCK_SIZE; j++) {
               sum = (sum>>1) + ((sum&1)<<15);
               sum += buf[j];
               sum &= 0xffff;
            }
         }
         if(sum == data[2]) {
            UART_Transmit("checksum ok", 1);
            for(error=0, i=0; i<(data[1]>>9) && !error; i++) {
               fs_fileread_datapart(file, 12+i*FW_BLOCK_SIZE, FW_BLOCK_SIZE, buf);
               //res = iap_erase_page((FW_FLASH_ADDR+i*FW_BLOCK_SIZE)>>6, (FW_FLASH_ADDR+i*FW_BLOCK_SIZE+FW_BLOCK_SIZE-1)>>6);
               res = iap_erase_sectors((FW_FLASH_ADDR+i*FW_BLOCK_SIZE)>>10, (FW_FLASH_ADDR+i*FW_BLOCK_SIZE+FW_BLOCK_SIZE-1)>>10);
               if(res == IAP_CMD_SUCCESS) {
                  res = iap_copy_ram_to_flash(FW_FLASH_ADDR+i*FW_BLOCK_SIZE, buf, FW_BLOCK_SIZE);
                  if(res == IAP_CMD_SUCCESS) {
                     res = iap_compare(FW_FLASH_ADDR+i*FW_BLOCK_SIZE, (unsigned int)buf, FW_BLOCK_SIZE);
                     if(res == IAP_CMD_SUCCESS) {
                        UART_Transmit(".", 0);
                     }
                     else {
                        UART_Transmit("compare error", 1);
                        error = 1;
                     }
                  }
                  else {
                     UART_Transmit("copy to flash error", 1);
                     error = 1;
                  }
               }
               else {
                  UART_Transmit("erase sector error", 1);
                  error = 1;
               }
            }
            if(!error) {
               UART_Transmit("ok", 1);
            }
         }
         else {
            UART_Transmit("checksum error", 1);
            error = 2;
         }
      }
      else {
         UART_Transmit("firmware file error", 1);
         error = 2;
      }
      if(error==0 || error == 2) {
         fs_filedelete(file);
         fs_flush();
      }
   }

   _set_msp(*((unsigned int*)FW_FLASH_ADDR));
   VTOR = (FW_FLASH_ADDR);
   _dsb();
   ((void(*)(void))(*((unsigned int*)(FW_FLASH_ADDR+4))))();
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
