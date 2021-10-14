#ifndef _insertion_kernel_lib_thread_locking_h
#define _insertion_kernel_lib_thread_locking_h

#include "./../inc/types.h"

typedef sint32_t ksemaphore_t;

void ksem_wait(ksemaphore_t* sem_ptr);
void ksem_post(ksemaphore_t* sem_ptr);
void ksem_init(ksemaphore_t* sem_ptr);

#endif
