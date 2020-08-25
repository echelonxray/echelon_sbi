#ifndef _insertion_kernel_lib_memalloc_h
#define _insertion_kernel_lib_memalloc_h

#include "./../inc/types.h"

void* kmalloc(size_t size);
void kfree(void* ptr);

#endif
