#ifndef _insertion_string_h
#define _insertion_string_h

#include <inc/types.h>

void memset(void* s, unsigned int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
size_t strlen(const char* str);
size_t strnlen(const char* str, size_t maxlen);
char* strcpy(char* dest, char* src);
signed int strcmp(const char *s1, const char *s2);
signed int strncmp(const char *s1, const char *s2, size_t n);

#endif
