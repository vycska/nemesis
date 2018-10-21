#include "output.h"
#include "ds3231.h"
#include "file_system.h"
#include "uart.h"
#include "utils.h"
#include "utils-asm.h"
#include "lpc824.h"
#include <string.h>
#include <time.h>

struct Output_Data output_data;

void output(char *buf, enum eOutputSubsystem subsystem, enum eOutputLevel level) {
   char *p, date[20];
   int i, l, len, file;
   struct tm dt;
   for(i=0; i<eOutputChannelLast; i++)
      if((output_data.channel_mask & (1u<<i))==0 && (output_data.subsystem_mask[subsystem] & (1u<<level))==0)
         switch(i) {
            case eOutputChannelUART:
               UART_Transmit(buf,strlen(buf),1);
               break;
            case eOutputChannelFile:
               if((file = fs_filesearch(OUTPUT_FILE_NAME)) == STATUS_ERROR) {
                  file = fs_filenew(OUTPUT_FILE_NAME, 100, 1);
                  fs_flush();
               }
               if(file != STATUS_ERROR) {
                  DS3231_GetDate(&dt);
                  for(len=mysprintf(date, "%d-%d-%d,%d:%d:%d",dt.tm_year+1900,dt.tm_mon+1,dt.tm_mday,dt.tm_hour,dt.tm_min,dt.tm_sec),p=date,i=0; i<4; (i+=1),p=(i==1?"\r\n":(i==2?buf:"\r\n\r\n")),len=(i==1?2:(i==2?strlen(buf):4))) {
                     while(len>0) {
                        l = MIN2(OUTPUT_FILE_BUFFER_SIZE-output_data.file_data_len, len);
                        memcpy(output_data.file_data+output_data.file_data_len, p, l);
                        output_data.file_data_len += l;
                        p += l;
                        len -= l;
                        if(output_data.file_data_len == OUTPUT_FILE_BUFFER_SIZE) {
                           fs_fileappend(file, output_data.file_data, OUTPUT_FILE_BUFFER_SIZE);
                           output_data.file_data_len = 0;
                        }
                     }
                  }
               }
               break;
         }
}
