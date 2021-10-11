#ifndef _insertion_kernel_lib_memalloc_h
#define _insertion_kernel_lib_memalloc_h

#include "./../inc/types.h"

void kalloc_pageinit(void* block_start, void* block_end);
void kfree_page(void* ptr);
void* kalloc_page();

void kfree_stack(void* ptr);
void* kmalloc_stack(size_t size);

void kallocinit(void* block_start, void* block_end);
void kfree(void* ptr);
void* kmalloc(size_t size);

#endif
