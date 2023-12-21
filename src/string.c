#include "string.h"

void memset(void* s, unsigned int c, size_t n) {
	unsigned char* ptr;
	unsigned char* end_addr;
	unsigned char value;
	ptr = s;
	end_addr = s + n;
	value = (unsigned char)(c & 0xFF);
	while (ptr < end_addr) {
		*ptr = value;
		ptr++;
	}
	return;
}

void *memcpy(void *dest, const void *src, size_t n) {
	const unsigned char* ptr_src = src;
	unsigned char* ptr_dest = dest;
	unsigned char* dest_end = dest + n;
	
	while (ptr_dest != dest_end) {
		*ptr_dest = *ptr_src;
		ptr_src++;
		ptr_dest++;
	}
	
	return dest;
}

size_t strlen(const char* str) {
	size_t i;
	i = 0;
	while (str[i] != 0) {
		i++;
	}
	return i;
}
size_t strnlen(const char* str, size_t maxlen) {
	size_t i;
	i = 0;
	while (i < maxlen) {
		if (str[i] == 0) {
			break;
		}
		i++;
	}
	return i;
}

char* strcpy(char* dest, char* src) {
	size_t i;
	i = 0;
	while (src[i] != 0) {
		dest[i] = src[i];
		i++;
	}
	dest[i] = src[i];
	return dest;
}

signed int strcmp(const char *s1, const char *s2) {
	unsigned int i = 0;
	while (s1[i] == s2[i]) {
		if (s1[i] == 0) {
			return 0;
		}
		i++;
	}
	return 1;
}
signed int strncmp(const char *s1, const char *s2, size_t n) {
	size_t i = 0;
	while (i < n) {
		if (s1[i] == s2[i]) {
			if (s1[i] == 0) {
				return 0;
			}
		} else {
			return 1;
		}
		i++;
	}
	return 0;
}
