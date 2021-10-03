#include "./string.h"

void itoa(register unsigned int number, register char* buf, register signed int buf_len, register signed int base, register signed int set_width) {
	//register signed long int num;
	register unsigned int num;
	num = number;
	if (base < 0) {
		base = -base;
		/*
		if(number < 0) {
			num = -num - 1;
		}
		*/
	}
	if (base > 16 || base < 2) {
		return;
	}
	if (buf_len < 2) {
		return;
	}
	if (set_width != 0) {
		if (set_width < 0 || num >= 0) {
			if (buf_len <= set_width) {
				return;
			}
		} else {
			if (buf_len <= set_width + 1) {
				return;
			}
		}
	}
	//Check if zero since this will fail to loop and can be easily handled now
	register unsigned char i_2;
	if (num == 0) {
		if (set_width != 0) {
			if (set_width < 0) {
				set_width = -set_width;
			}
			i_2 = 0;
			while (i_2 < set_width) {
				buf[i_2] = '0';
				i_2++;
			}
			buf[i_2] = 0;
			return;
		} else {
			buf[0] = '0';
			buf[1] = 0;
			return;
		}
	} else {
		register unsigned char i_then_length;
		i_then_length = 0;
		i_2 = 0;
		//i_then_length is now an index
		//Append "-" character for negatives
		if(num < 0){
			if (set_width < 0) {
				set_width++;
			}
			num = -num;
			buf[0] = '-';
			buf_len--;
			buf++;
		}
		if (set_width < 0) {
			set_width = -set_width;
		}
		//Find Characters
		while (num > 0 && i_then_length < buf_len) {
			i_2 = num % base;
			if (i_2 < 10) {
				buf[(unsigned int)i_then_length] = '0' + i_2;
			} else {
				buf[(unsigned int)i_then_length] = '7' + i_2;
			}
			num /= base;
			i_then_length++;
		}
		while (i_then_length < set_width && i_then_length < buf_len) {
			buf[(unsigned int)i_then_length] = '0';
			i_then_length++;
		}
		//i_then_length is now a length count for char array
		//Loop to fix character order
		i_2 = 0;
		register char tmpchar;
		while (i_2 < (i_then_length / 2) && i_2 < buf_len) {
			tmpchar = buf[(int)((i_then_length - i_2) - 1)];
			buf[(int)((i_then_length - i_2) - 1)] = buf[(unsigned int)i_2];
			buf[(unsigned int)i_2] = tmpchar;
			i_2++;
		}
		if (i_then_length < buf_len) {
			buf[(unsigned int)i_then_length] = 0;
		} else {
			buf[(unsigned int)(i_then_length - 1)] = 0;
		}
	}
	return;

}
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
	while (n > 0) {
		*ptr_dest = *ptr_src;
		ptr_dest++;
		ptr_src++;
		n--;
	}
	return dest;
}

size_t strlen(char* str) {
	size_t i;
	i = 0;
	while (str[i] != 0) {
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
