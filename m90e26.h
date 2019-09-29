#include <stdbool.h>

#define M90E26_START_MARKER 0xFE

void (*m90e26_uart_putc)(unsigned char);
unsigned char (*m90e26_uart_getc)();

bool m90e26_read_reg(unsigned char address, unsigned int *data);
bool m90e26_write_reg(unsigned char address, unsigned int data);
