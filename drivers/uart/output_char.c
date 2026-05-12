#include "board.h"

void output_char(char c)
{
    volatile unsigned int *uart = (volatile unsigned int *)UART_BASE;

    /* Wait until UART TX FIFO is not full (TXFF flag) */
    while (*(uart + (UART_FR / 4)) & UART_FR_TXFF);

    /* Write the character to UART data register */
    *(uart + (UART_DR / 4)) = c;
}
