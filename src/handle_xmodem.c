#include "handle_xmodem.h"
#include "handle_log.h"
#include "fifos.h"
#include "file_system.h"
#include "uart.h"
#include "utils.h"
#include "pt.h"
#include "timer.h"

struct pt pt_xmodem_sending,
          pt_xmodem_receiving;
static struct timer timer_xmodem_sending,
                    timer_xmodem_receiving;
struct Handle_Xmodem_Data handle_xmodem_data;

PT_THREAD(Handle_Xmodem_Sending(struct pt *pt)) {
   unsigned char received_data;
   unsigned int i, s;
   static unsigned char packet[132];
   static int error,
              retry,
              block,
              data_length,
              file_read,
              file_size;
   PT_BEGIN(pt);
   if(Fifo_Xmodem_Sending_Get(&received_data)==1 && received_data==NAK && Fifo_Xmodem_Sending_Get(&received_data)==0) {
      for(error=0,block=1,file_read=0,file_size=(!fs_direntryempty(handle_xmodem_data.sending_file)?fs_filesize(handle_xmodem_data.sending_file):0); file_read<=file_size && !error; block=(block+1)&0xff, file_read+=data_length) {
         if(file_read<file_size) {
            packet[0] = SOH;
            packet[1] = block;
            packet[2] = 255-block;
            data_length = MIN2(128, file_size-file_read);
            fs_fileread_seq(handle_xmodem_data.sending_file, &packet[3], data_length);
            for(i=3+data_length; i<=130; i++)
               packet[i] = SUB;
            for(s=i=0; i<=130;i++)
               s = (s+packet[i]) & 0xff;
            packet[131] = s;
         }
         else {
            packet[0] = EOT;
         }
         for(received_data=0,retry=0; retry<10 && received_data!=ACK && received_data!=CAN; retry++) {
            UART_Transmit((char*)packet,file_read==file_size?1:132,0);
            timer_set(&timer_xmodem_sending, 3+retry);
            PT_WAIT_UNTIL(&pt_xmodem_sending, Fifo_Xmodem_Sending_Get(&received_data)==1 || timer_expired(&timer_xmodem_sending));
         }
         if(retry==10 || received_data==CAN || Fifo_Xmodem_Sending_Get(&received_data)!=0) error = 1;
      }
   }
   fs_fileread_seq(STATUS_ERROR,0,0);
   PT_END(pt);
}

PT_THREAD(Handle_Xmodem_Receiving(struct pt *pt)) {
   unsigned int i, s;
   static unsigned char packet[2];
   static int state,
              packet_size,
              block,
              retry,
              file;
   PT_BEGIN(pt);
   
   file = fs_filenew(handle_xmodem_data.receiving_file, 0, 1);
   for(block=0,state=0; state<2; block = (block+1)&0xff) {
      //sitoj vietoje as tikrai turiu, kad gautas paketas [arba state==0]
      if(state!=0 && handle_xmodem_data.receiving_size == 132) {
         for(s=i=0;i<=130;i++)
            s = (s+handle_xmodem_data.receiving_data[i]) & 0xff;
         if(handle_xmodem_data.receiving_data[0]==SOH && handle_xmodem_data.receiving_data[1]==block && handle_xmodem_data.receiving_data[2]==255-block && handle_xmodem_data.receiving_data[131]==s) {
            fs_fileappend(file, (char*)&handle_xmodem_data.receiving_data[3], 128);
            state = 1;
         }
         else state = 3;
      }
      else if(state!=0) //kitas dydis yra 1 -- EOT arba CAN
         state = (handle_xmodem_data.receiving_data[0]==EOT ? 2 : 4);
      //state = 0 : startas, state = 1 : gautas blokas ir viskas ok, issiunciamas ACK; state = 2 : gautas EOT, issiunciamas ACK ir baigiama; state = 3 : kazkas negerai ir baigiu issiusdamas CAN; state = 4 : tiesiog baigiama
      if(file == STATUS_ERROR) state=3;
      if(state==0) {
         packet[0] = NAK;
         packet_size = 1;
         retry = 10;
         state = 1;
      }
      else if(state==1) {
         packet[0] = ACK;
         packet_size = 1;
         retry = 10;
      }
      else if(state==2) {
         packet[0] = ACK;
         packet_size = 1;
         retry = 1;
      }
      else if(state==3) {
         packet[0] = packet[1] = CAN;
         packet_size = 2;
         retry = 1;
      }
      else if(state==4) {
         retry = 0;
      }
      for(handle_xmodem_data.receiving_ready=0; retry>0 && handle_xmodem_data.receiving_ready==0; retry--) {
         UART_Transmit((char*)packet, packet_size, 0);
         timer_set(&timer_xmodem_receiving, 10);
         if(state<2)
            PT_WAIT_UNTIL(&pt_xmodem_receiving, handle_xmodem_data.receiving_ready==1 || timer_expired(&timer_xmodem_receiving));
      }
      if(retry==0 && state!=2) state = 4;
   }
   if(state != 2) fs_filedelete(file);
   fs_flush();

   PT_END(pt);
}
