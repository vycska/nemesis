#ifndef __FIFOS_H__
#define __FIFOS_H__

#define FIFO_COMMAND_PARSER_ITEMS 4
#define FIFO_COMMAND_PARSER_ITEM_SIZE 48
#define FIFO_UART_OUTPUT_ITEMS 16

struct Fifo_Command_Parser {
   char buffer[FIFO_COMMAND_PARSER_ITEMS][FIFO_COMMAND_PARSER_ITEM_SIZE];
   int i_get, i_put,
       mtx_fifo_command_parser, smphr_count_items;
};

struct Fifo_Uart_Output {
   char *buffer[FIFO_UART_OUTPUT_ITEMS];
   int i_get, i_put,
       mtx_fifo_uart_output, smphr_count_items, smphr_space_left, *smphr_finished[FIFO_UART_OUTPUT_ITEMS];
};

/* ********** */

void Fifo_Command_Parser_Init(void);
void Fifo_Command_Parser_Get(char **);
void Fifo_Command_Parser_Put(char *);

void Fifo_Uart_Output_Init(void);
void Fifo_Uart_Output_Get(char **, int **);
void Fifo_Uart_Output_Put(char *, int *);

#endif
