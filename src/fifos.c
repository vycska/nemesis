#include "fifos.h"
#include <string.h>

static struct Fifo_Command_Parser fifo_command_parser;

void Fifo_Command_Parser_Init(void) {
   fifo_command_parser.count = 0;
   fifo_command_parser.i_get = fifo_command_parser.i_put = 0;
}

int Fifo_Command_Parser_Get(char **pString) {
   int res;
   if(fifo_command_parser.count>0) {
      *pString = fifo_command_parser.buffer[fifo_command_parser.i_get];
      fifo_command_parser.i_get = (fifo_command_parser.i_get + 1) & (FIFO_COMMAND_PARSER_ITEMS-1);
      fifo_command_parser.count -= 1;
      res = 1;
   }
   else res = 0;
   return res;
}

void Fifo_Command_Parser_Put(char *pString) {
   if(fifo_command_parser.count<FIFO_COMMAND_PARSER_ITEMS) {
      strcpy(fifo_command_parser.buffer[fifo_command_parser.i_put], pString);
      fifo_command_parser.i_put = (fifo_command_parser.i_put + 1) & (FIFO_COMMAND_PARSER_ITEMS-1);
      fifo_command_parser.count += 1;
   }
}

