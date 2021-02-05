#ifndef _insertion_lib_itoa_h
#define _insertion_lib_itoa_h

#include "./types.h"

void itoa(register signed int number, register char* buf, register signed int buf_len, register signed int base, register signed int set_width);
void memset(void* s, unsigned int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(char* str);
char* strcpy(char* dest, char* src);
signed int strcmp(const char *s1, const char *s2);
signed int strncmp(const char *s1, const char *s2, size_t n);

#endif
