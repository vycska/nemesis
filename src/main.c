#include "main.h"
#include "adc.h"
#include "bme280.h"
#include "config.h"
#include "ds18b20.h"
#include "ds3231.h"
#include "fifos.h"
#include "file_system.h"
#include "handle_command.h"
#include "handle_log.h"
#include "handle_measurements.h"
#include "handle_xmodem.h"
#include "i2c.h"
#include "iap.h"
#include "led.h"
#include "mrt.h"
#include "output.h"
#include "pll.h"
#include "spi.h"
#include "switch.h"
#include "uart.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"
#include "pt.h"

extern char _data_start_lma, _data_start, _data_end, _bss_start, _bss_end;
extern char _flash_start, _flash_end, _ram_start, _ram_end;
extern char _intvecs_size, _text_size, _rodata_size, _data_size, _bss_size, _stack_size, _heap_size;

extern struct pt pt_xmodem_sending,
                 pt_xmodem_receiving;
extern struct UART_Data uart_data;
extern volatile struct Switch_Data switch_data;

unsigned int start_time;
volatile unsigned int gInterruptCause = 0;

void main(void) {
   char *command,
        buf[64];
   unsigned char data[8];
   int i, l, t;
   unsigned int cause;

   t = config_load();

   PDRUNCFG &= (~(1<<0 | 1<<1 | 1<<2 | 1<<4 | 1<<7)); //IRC output, IRC, flash, ADC, PLL powered
   SYSAHBCLKCTRL |= (1<<1 | 1<<2 | 1<<3 | 1<<4 | 1<<5 | 1<<6 | 1<<7 | 1<<10 | 1<<11 | 1<<14 | 1<<18 | 1<<24); //enable clock for ROM, RAM0_1, FLASHREG, FLASH, I2C0, GPIO, SWM, MRT, SPI0, USART0, IOCON, ADC
   PRESETCTRL |= (1<<0 | 1<<2 | 1<<3 | 1<<6 | 1<<7 | 1<<10 | 1<<11 | 1<<24); //clear SPI0, USART FRG, USART0, I2C0, MRT, GPIO, flash controller, ADC reset

   PLL_Init();
   ADC_Init();
   I2C0_Init();
   LED_Init(1);
   SPI0_Init();
   UART_Init();

   MRT_Init();

   DS3231_Init();
   Switch_Init();

   //DS18B20
   DS18B20_Init();
   if(DS18B20_ReadROM(data) == DS18B20_OK) {
      l = mysprintf(buf, "one-wire device: ");
      for(i = 0; i < 8; i++)
         l += mysprintf(&buf[l], "0x%x%s", (unsigned int)data[i], i == 7 ? "." : " ");
      output(buf, eOutputSubsystemDS18B20, eOutputLevelNormal);
   }

   //BME280 [2ms max time until communication]
   BME280_Init();
   if(BME280_GetID(data)==1) {
      mysprintf(buf,"BME280 id: %x",(unsigned int)data[0]);
      output(buf, eOutputSubsystemBME280, eOutputLevelNormal);
   }

   fs_mount();
   if(fs_checkdisk()==STATUS_ERROR) {
      fs_format();
      output("fs_checkdisk error", eOutputSubsystemSystem, eOutputLevelImportant);
   }

   Fifo_Command_Parser_Init();
   Fifo_Xmodem_Sending_Init();
   PT_INIT(&pt_xmodem_sending);
   PT_INIT(&pt_xmodem_receiving);

   for(i=0; i<=15; i++) {
      switch(i) {
         case 0:
            mysprintf(buf,"VERSION: %d",VERSION);
            break;
         case 1:
            mysprintf(buf,"%s %s",__DATE__,__TIME__);
            break;
         case 2:
            mysprintf(buf,"%s",t?"config_load error":"config_load ok");
            break;
         case 3:
            mysprintf(buf, "_flash_start: %x [%u]", (unsigned int)&_flash_start,(unsigned int)&_flash_start);
            break;
         case 4:
            mysprintf(buf, "_flash_end: %x [%u]", (unsigned int)&_flash_end,(unsigned int)&_flash_end);
            break;
         case 5:
            mysprintf(buf, "_ram_start: %x [%u]", (unsigned int)&_ram_start,(unsigned int)&_ram_start);
            break;
         case 6:
            mysprintf(buf, "_ram_end: %x [%u]", (unsigned int)&_ram_end,(unsigned int)&_ram_end);
            break;
         case 7:
            mysprintf(buf, "_intvecs_size: %u", (unsigned int)&_intvecs_size);
            break;
         case 8:
            mysprintf(buf, "_text_size: %u", (unsigned int)&_text_size);
            break;
         case 9:
            mysprintf(buf, "_rodata_size: %u", (unsigned int)&_rodata_size);
            break;
         case 10:
            mysprintf(buf, "_data_size: %u", (unsigned int)&_data_size);
            break;
         case 11:
            mysprintf(buf, "_bss_size: %u", (unsigned int)&_bss_size);
            break;
         case 12:
            mysprintf(buf, "_stack_size: %u", (unsigned int)&_stack_size);
            break;
         case 13:
            mysprintf(buf, "_heap_size: %u", (unsigned int)&_heap_size);
            break;
         case 14:
            mysprintf(buf, "flash used: %u",(unsigned int)&_intvecs_size+(unsigned int)&_text_size+(unsigned int)&_rodata_size+(unsigned int)&_data_size);
            break;
         case 15:
            mysprintf(buf, "ram used: %u",(unsigned int)&_data_size+(unsigned int)&_bss_size+(unsigned int)&_stack_size+(unsigned int)&_heap_size);
            break;
      }
      output(buf, eOutputSubsystemSystem, eOutputLevelImportant);
   }

   while(1) {
      cause = gInterruptCause; //atomic interrupt safe read
      while(cause) {
         if(cause & (1<<0)) { //button pressed and no deep-sleep mode active
            if(Fifo_Command_Parser_Get(&command))
               Handle_Command(command);
         }
         if(cause & (1<<1)) { //alarm1
            DisableInterrupts();
            gInterruptCause &= (~(0x1<<1));
            EnableInterrupts();
            Handle_Measurements();
            Handle_Log();
         }
         if(cause & (1<<2)) { //alarm2
            DisableInterrupts();
            gInterruptCause &= (~(0x1<<2));
            EnableInterrupts();
         }
         if(cause & (1<<3)) { //button released
            DisableInterrupts();
            gInterruptCause &= (~(0x1<<3));
            EnableInterrupts();
            mysprintf(buf, "switch %d",switch_data.duration);
            output(buf, eOutputSubsystemSwitch, eOutputLevelDebug);
         }
         if(cause & (1<<4)) { //xmodem sending file
            if(PT_SCHEDULE(Handle_Xmodem_Sending(&pt_xmodem_sending))==0) {
               uart_data.i = 0;
               uart_data.mode = 0;
               DisableInterrupts();
               gInterruptCause &= (~(0x1<<4));
               EnableInterrupts();
            }
         }
         if(cause & (1<<5)) { //xmodem receiving file
            if(PT_SCHEDULE(Handle_Xmodem_Receiving(&pt_xmodem_receiving))==0) {
               uart_data.i = 0;
               uart_data.mode = 0;
               DisableInterrupts();
               gInterruptCause &= (~(0x1<<5));
               EnableInterrupts();
            }
         }
         cause = gInterruptCause;
      }
      DisableInterrupts();
      if(gInterruptCause == 0 && !switch_data.active) {
         EnableInterrupts();
         LED_Off();
         //go to deep sleep
         PCON = (PCON & (~(0x7<<0))) | (1<<0); //deep-sleep mode
         PDSLEEPCFG |= (1<<3 | 1<<6); //BOD for deep-sleep powered down, watchdog oscillator for deep-sleep powered down
         PDAWAKECFG = (0<<0 | 0<<1 | 0<<2 | 1<<3 | 0<<4 | 1<<5 | 1<<6 | 0<<7 | 0xd<<8 | 0x6<<12 | 1<<15); //power configuration after wake up: IRC oscillator output powered, IRC oscillator power-down powered, flash powered, BOD powered down, ADC powered, crystal oscillator powered down, watchdog oscillator powered down, system PLL powered, ACMP powered down
         STARTERP0 = (1<<0 | 1<<1); //GPIO pint interrupt 0 and 1 wake-up enabled
         STARTERP1 = 0; //all other interrupts for wake-up disabled
         MAINCLKSEL = 0; //clock source for main clock is IRC
         MAINCLKUEN = 0;
         MAINCLKUEN = 1; //update clock source
         SCR = (SCR&(~(0x1<<1 | 0x1<<2))) | (0<<1 | 1<<2); //do not sleep when returning to thread mode, deep sleep is processor's low power mode
         WaitForInterrupt();
         //returned form deep sleep
         MAINCLKSEL = 0x3; //clock source for main clock is PLL output
         MAINCLKUEN = 0;
         MAINCLKUEN = 1; //update clock source
         PCON &= (~(0x7<<0));
         LED_On();
      }
      EnableInterrupts();
   }
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

void SystemReset(void) {
   _DSB();
   AIRCR = (AIRCR & (~(1<<1 | 1<<2 | 0xffffu<<16))) | (1<<2 | 0x5fau<<16);
   _DSB();
   while(1);
}
