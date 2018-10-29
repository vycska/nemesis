#ifndef __FIFOS_H__
#define __FIFOS_H__

#define FIFO_COMMAND_PARSER_ITEMS 4
#define FIFO_COMMAND_PARSER_ITEM_SIZE 64
#define FIFO_XMODEM_SENDING_ITEMS 16

struct Fifo_Command_Parser {
   volatile char buffer[FIFO_COMMAND_PARSER_ITEMS][FIFO_COMMAND_PARSER_ITEM_SIZE];
   volatile int count;
   int i_get, i_put;
};

struct Fifo_Xmodem_Sending {
   volatile char data[FIFO_XMODEM_SENDING_ITEMS];
   volatile int count;
   int i_get, i_put;
};

void Fifo_Command_Parser_Init(void);
int Fifo_Command_Parser_Get(char **);
void Fifo_Command_Parser_Put(char *);

void Fifo_Xmodem_Sending_Init(void);
int Fifo_Xmodem_Sending_Get(unsigned char *);
void Fifo_Xmodem_Sending_Put(unsigned char);

#endif
