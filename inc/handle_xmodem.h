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

enum XmodemReceiveState {
   eXmodemReceiveStateStartNAK        = 0,
   eXmodemReceiveStateReceivedBlock   = 1,
   eXmodemReceiveStateReceivedEOT     = 2,
   eXmodemReceiveStateCancel          = 3,
   eXmodemReceiveStateExit            = 4
};

struct Handle_Xmodem_Data {
   char receiving_file[MAX_FILE_NAME_SIZE];
   volatile unsigned char receiving_data[132];
   volatile int receiving_ready,
                receiving_size,
                receiving_lost;
   int sending_file;
};

PT_THREAD(Handle_Xmodem_Sending(struct pt *pt));
PT_THREAD(Handle_Xmodem_Receiving(struct pt *pt));

#endif
