#include "fifos.h"
#include "os.h"
#include <string.h>

struct Fifo_Command_Parser fifo_command_parser;
struct Fifo_Uart_Output fifo_uart_output;

void Fifo_Command_Parser_Init(void) {
   fifo_command_parser.i_get = fifo_command_parser.i_put = 0;
   Semaphore_Init(&fifo_command_parser.mtx_fifo_command_parser, 1);
   Semaphore_Init(&fifo_command_parser.smphr_count_items, 0);
}

void Fifo_Command_Parser_Get(char **pString) {
   Task_Blocking_Wait(&fifo_command_parser.smphr_count_items);
   Task_Blocking_Wait(&fifo_command_parser.mtx_fifo_command_parser);

   *pString = fifo_command_parser.buffer[fifo_command_parser.i_get];
   fifo_command_parser.i_get = (fifo_command_parser.i_get + 1) & (FIFO_COMMAND_PARSER_ITEMS-1);

   Task_Blocking_Signal(&fifo_command_parser.mtx_fifo_command_parser);
}

void Fifo_Command_Parser_Put(char *pString) {
   Task_Blocking_Wait(&fifo_command_parser.mtx_fifo_command_parser);

   strcpy(fifo_command_parser.buffer[fifo_command_parser.i_put], pString);
   fifo_command_parser.i_put = (fifo_command_parser.i_put + 1) & (FIFO_COMMAND_PARSER_ITEMS-1);

   Task_Blocking_Signal(&fifo_command_parser.mtx_fifo_command_parser);
   Task_Blocking_Signal(&fifo_command_parser.smphr_count_items);
}

/* ********** */

void Fifo_Uart_Output_Init(void) {
   fifo_uart_output.i_get = fifo_uart_output.i_put = 0;
   Semaphore_Init(&fifo_uart_output.mtx_fifo_uart_output, 1);
   Semaphore_Init(&fifo_uart_output.smphr_count_items, 0);
   Semaphore_Init(&fifo_uart_output.smphr_space_left, FIFO_UART_OUTPUT_ITEMS);
}

void Fifo_Uart_Output_Get(char **pString, int **smphrFinished) {
   Task_Blocking_Wait(&fifo_uart_output.smphr_count_items);
   Task_Blocking_Wait(&fifo_uart_output.mtx_fifo_uart_output);

   *pString = fifo_uart_output.buffer[fifo_uart_output.i_get];
   *smphrFinished = fifo_uart_output.smphr_finished[fifo_uart_output.i_get];
   fifo_uart_output.i_get = (fifo_uart_output.i_get + 1) & (FIFO_UART_OUTPUT_ITEMS-1);

   Task_Blocking_Signal(&fifo_uart_output.mtx_fifo_uart_output);
   Task_Blocking_Signal(&fifo_uart_output.smphr_space_left);
}

void Fifo_Uart_Output_Put(char *pString, int *smphrFinished) {
   Task_Blocking_Wait(&fifo_uart_output.smphr_space_left);
   Task_Blocking_Wait(&fifo_uart_output.mtx_fifo_uart_output);

   fifo_uart_output.buffer[fifo_uart_output.i_put] = pString;
   fifo_uart_output.smphr_finished[fifo_uart_output.i_put] = smphrFinished;
   fifo_uart_output.i_put = (fifo_uart_output.i_put + 1) & (FIFO_UART_OUTPUT_ITEMS-1);

   Task_Blocking_Signal(&fifo_uart_output.mtx_fifo_uart_output);
   Task_Blocking_Signal(&fifo_uart_output.smphr_count_items);
}
