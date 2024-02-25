#ifndef _insertion_entry_h
#define _insertion_entry_h

#include <inc/types.h>

void idle_loop() __attribute__((noreturn));
unsigned long probe_csr(unsigned long csr_address);

#endif
