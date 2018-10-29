#include "fifos.h"
#include <string.h>

static struct Fifo_Command_Parser fifo_command_parser;
static struct Fifo_Xmodem_Sending fifo_xmodem_sending;

void Fifo_Command_Parser_Init(void) {
   fifo_command_parser.count = 0;
   fifo_command_parser.i_get = 0;
   fifo_command_parser.i_put = 0;
}

int Fifo_Command_Parser_Get(char **pString) {
   int res;
   if(fifo_command_parser.count>0) {
      *pString = (char*)fifo_command_parser.buffer[fifo_command_parser.i_get];
      fifo_command_parser.i_get = (fifo_command_parser.i_get + 1) & (FIFO_COMMAND_PARSER_ITEMS-1);
      fifo_command_parser.count -= 1;
      res = 1;
   }
   else res = 0;
   return res;
}

void Fifo_Command_Parser_Put(char *pString) {
   if(fifo_command_parser.count<FIFO_COMMAND_PARSER_ITEMS) {
      strcpy((char*)fifo_command_parser.buffer[fifo_command_parser.i_put], pString);
      fifo_command_parser.i_put = (fifo_command_parser.i_put + 1) & (FIFO_COMMAND_PARSER_ITEMS-1);
      fifo_command_parser.count += 1;
   }
}

void Fifo_Xmodem_Sending_Init(void) {
   fifo_xmodem_sending.count = 0;
   fifo_xmodem_sending.i_get = 0;
   fifo_xmodem_sending.i_put = 0;
}

int Fifo_Xmodem_Sending_Get(unsigned char *data) {
   int res;
   if(fifo_xmodem_sending.count>0) {
      *data = fifo_xmodem_sending.data[fifo_xmodem_sending.i_get];
      fifo_xmodem_sending.i_get = (fifo_xmodem_sending.i_get + 1) & (FIFO_XMODEM_SENDING_ITEMS-1);
      fifo_xmodem_sending.count -= 1;
      res = 1;
   }
   else res = 0;
   return res;
}

void Fifo_Xmodem_Sending_Put(unsigned char data) {
   if(fifo_xmodem_sending.count < FIFO_XMODEM_SENDING_ITEMS) {
      fifo_xmodem_sending.data[fifo_xmodem_sending.i_put] = data;
      fifo_xmodem_sending.i_put = (fifo_xmodem_sending.i_put + 1 ) & (FIFO_XMODEM_SENDING_ITEMS-1);
      fifo_xmodem_sending.count += 1;
   }
}
