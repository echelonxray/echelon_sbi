#include "./string.h"

void itoa(uintRL_t num, char* buf, size_t buf_len, signed int base, signed int set_min_width) {
	// At least write a NULL terminator so that if the calling function 
	// attempts to use the buffer, it will not run off into memory.
	if (buf_len > 0) {
		*buf = 0;
	}
	
	// Verify that the buffer length is long enough to be useful.
	if (buf_len < 3) {
		return;
	}
	
	// If set_min_width is 0, don't pad zeros.
	// 
	// Otherwise, '0's are padded on left to meet a minimum
	// length of set_min_width.  The behaviour is as described:
	// 
	// If base is positive:
	// The sign of the number is considered part of the length.
	// However, it is not padded with '0'.  Rather, if
	// set_min_width is positive, number is non-negative,
	// and padding is required, a single space of padding
	// is added where the sign character would be.  If
	// set_min_width were negative given the same situation, a '+'
	// is used to prefix a non-negative number.  If number is
	// negative, '-' is always used as a prefix and the sign of
	// set_min_width makes no difference.
	// 
	// If base is negative:
	// The sign of set_min_width is ignored and the magnitude of
	// set_min_width is used as a the minimum length.  '0's are
	// padded to the left of number to meet the minimum length.
	
	{
		char pos_sign;
		pos_sign = ' ';
		if (set_min_width < 0) {
			pos_sign = '+';
			set_min_width = -set_min_width;
		}
		if (set_min_width != 0) {
			if (buf_len <= (size_t)set_min_width) {
				// Buffer is too short to encode to the requested size
				// number and add the NULL terminator.
				return;
			}
			if (base >= 0) {
				if (set_min_width == 1) {
					// This doesn't make sense.  set_min_width is too short.
					return;
				}
				buf_len--;
				set_min_width--;
				sintRL_t number = (sintRL_t)num;
				if (number < 0) {
					*buf = '-';
					num = (uintRL_t)(-number);
				} else {
					*buf = pos_sign;
				}
				buf++;
			} else {
				base = -base;
			}
		} else {
			if (base >= 0) {
				sintRL_t number = (sintRL_t)num;
				if (number < 0) {
					*buf = '-';
					num = (uintRL_t)(-number);
					buf++;
					buf_len--;
				}
			} else {
				base = -base;
			}
		}
	}
	
	// Verify the base is within valid range
	if (base > 16 || base < 2) {
		return;
	}
	
	// Exclude the NULL terminator from the buffer length so
	// that is can be easily looped.
	buf_len--;
	
	// Check if num is zero since this will fail to loop and
	// can be easily handled now.
	if (num == 0) {
		if (set_min_width != 0) {
			size_t i;
			for (i = 0; i < buf_len; i++) {
				buf[i] = '0';
			}
			buf[i] = 0;
			return;
		} else {
			buf[0] = '0';
			buf[1] = 0;
			return;
		}
		// Here is unreachable
	} else {
		size_t i_then_length = 0;
		// i_then_length is now an index
		// Find Characters
		while (num > 0 && i_then_length < buf_len) {
			uintRL_t number;
			number = num % base;
			if (number < 10) {
				buf[i_then_length] = (char)('0' + number);
			} else {
				buf[i_then_length] = (char)('7' + number);
			}
			num /= base;
			i_then_length++;
		}
		while (i_then_length < (size_t)set_min_width && i_then_length < buf_len) {
			buf[i_then_length] = '0';
			i_then_length++;
		}
		
		// i_then_length is now a length
		// Loop to reverse character order
		size_t i = 0;
		char tmpchar;
		while (i < (i_then_length / 2)) {
			tmpchar = buf[(i_then_length - i) - 1];
			buf[(i_then_length - i) - 1] = buf[i];
			buf[i] = tmpchar;
			i++;
		}
		buf[i_then_length] = 0;
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
