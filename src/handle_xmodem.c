#include "handle_xmodem.h"
#include "handle_log.h"
#include "fifos.h"
#include "file_system.h"
#include "uart.h"
#include "utils.h"
#include "pt.h"
#include "timer.h"

struct pt pt_xmodem_sending, pt_xmodem_receiving;
static struct timer timer_xmodem;
struct Handle_Xmodem_Data handle_xmodem_data;

PT_THREAD(Handle_Xmodem_Sending(struct pt *pt)) {
   unsigned char received_data;
   unsigned int i, s;
   static unsigned char packet[132];
   static int end,
              retry,
              block,
              data_length,
              file_read,
              file_size;
   PT_BEGIN(pt);
   timer_set(&timer_xmodem, 100);
   PT_WAIT_UNTIL(&pt_xmodem_sending, (received_data=0,(Fifo_Xmodem_Sending_Get(&received_data)==1 && received_data==NAK)) || timer_expired(&timer_xmodem));
   if(received_data==NAK) {
      for(end=0,block=1,file_read=0,file_size=(!fs_direntryempty(handle_xmodem_data.sending_file)?fs_filesize(handle_xmodem_data.sending_file):0); file_read<=file_size && !end; block=(block+1)&0xff, file_read+=data_length) {
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
            UART_Transmit((char*)packet, file_read==file_size?1:132, 0);
            timer_set(&timer_xmodem, 10);
            PT_WAIT_UNTIL(&pt_xmodem_sending, (received_data=0, Fifo_Xmodem_Sending_Get(&received_data)==1) || timer_expired(&timer_xmodem));
         }
         if(received_data != ACK) end = 1;
      }
   }
   fs_fileread_seq(STATUS_ERROR,0,0);
   PT_END(pt);
}

PT_THREAD(Handle_Xmodem_Receiving(struct pt *pt)) {
   unsigned int i, s;
   static unsigned char packet[2];
   static enum XmodemReceiveState state;
   static int packet_size,
              block,
              retry,
              file;
   PT_BEGIN(pt);
   file = fs_filenew(handle_xmodem_data.receiving_file, 0, 1);
   for(block=0,state=eXmodemReceiveStateStartNAK; state==eXmodemReceiveStateStartNAK || state==eXmodemReceiveStateReceivedBlock; block = (block+1)&0xff) {
      if(state!=eXmodemReceiveStateStartNAK) {
         if(handle_xmodem_data.receiving_size == 132) {
            for(s=i=0;i<=130;i++)
               s = (s+handle_xmodem_data.receiving_data[i]) & 0xff;
            if(handle_xmodem_data.receiving_data[0]==SOH && handle_xmodem_data.receiving_data[1]==block && handle_xmodem_data.receiving_data[2]==255-block && handle_xmodem_data.receiving_data[131]==s) {
               fs_fileappend(file, (char*)&handle_xmodem_data.receiving_data[3], 128);
               state = eXmodemReceiveStateReceivedBlock;
            }
            else state = eXmodemReceiveStateCancel;
         }
         else //kitas dydis yra 1 -- EOT arba CAN
            state = (handle_xmodem_data.receiving_data[0]==EOT ? eXmodemReceiveStateReceivedEOT : eXmodemReceiveStateExit);
      }
      if(file == STATUS_ERROR) state=eXmodemReceiveStateCancel;
      switch(state) {
         case eXmodemReceiveStateStartNAK:
            packet[0] = NAK;
            packet_size = 1;
            retry = 10;
            break;
         case eXmodemReceiveStateReceivedBlock:
            packet[0] = ACK;
            packet_size = 1;
            retry = 10;
            break;
         case eXmodemReceiveStateReceivedEOT:
            packet[0] = ACK;
            packet_size = 1;
            retry = 1;
            break;
         case eXmodemReceiveStateCancel:
            packet[0] = packet[1] = CAN;
            packet_size = 2;
            retry = 1;
            break;
         case eXmodemReceiveStateExit:
            retry = 0;
            break;
      }
      for(handle_xmodem_data.receiving_ready=0; retry>0 && handle_xmodem_data.receiving_ready==0; retry--) {
         UART_Transmit((char*)packet, packet_size, 0);
         timer_set(&timer_xmodem, 10);
         if(state==eXmodemReceiveStateStartNAK || state==eXmodemReceiveStateReceivedBlock)
            PT_WAIT_UNTIL(&pt_xmodem_receiving, handle_xmodem_data.receiving_ready==1 || timer_expired(&timer_xmodem));
      }
      switch(state) {
         case eXmodemReceiveStateStartNAK:
            if(handle_xmodem_data.receiving_ready)
               state=eXmodemReceiveStateReceivedBlock;
            else state=eXmodemReceiveStateExit;
            break;
         case eXmodemReceiveStateReceivedBlock:
            if(!handle_xmodem_data.receiving_ready)
               state=eXmodemReceiveStateExit;
            break;
         case eXmodemReceiveStateReceivedEOT:
         case eXmodemReceiveStateCancel:
         case eXmodemReceiveStateExit:
            break;
      }
   }
   if(state != eXmodemReceiveStateReceivedEOT) fs_filedelete(file);
   fs_flush();
   PT_END(pt);
}
