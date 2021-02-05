#ifndef _insertion_kernel_drivers_uart_h
#define _insertion_kernel_drivers_uart_h

#include "./../../inc/types.h"

void uart_write(unsigned char* str, void* uart_loc, size_t count);

#endif
