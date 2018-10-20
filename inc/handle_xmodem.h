#ifndef __HANDLE_XMODEM_H__
#define __HANDLE_XMODEM_H__

#include "file_system.h"
#include "pt.h"

#define SOH 0x1 //start of header
#define EOT 0x4 //end of transmission
#define ACK 0x6 //acknowledge
#define NAK 0x15 //not acknowledge
#define CAN 0x18 //cancel
#define SUB 0x1a //substitute

struct Handle_Xmodem_Data {
   char receiving_file[MAX_FILE_NAME_SIZE];
   unsigned char receiving_data[132];
   int receiving_lost,
       receiving_size,
       receiving_ready;
   int sending_file;
};

PT_THREAD(Handle_Xmodem_Sending(struct pt *pt));
PT_THREAD(Handle_Xmodem_Receiving(struct pt *pt));

#endif
