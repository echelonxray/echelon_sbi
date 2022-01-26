#include "./string.h"

/*
signed long __printm(signed int fd, const char *format, unsigned long* gen_regs, long double* vec_regs, void* stack_params, unsigned long vector_regs_used) {
	// Parse String
	//void* stack_ptr = stack_params;
	char buff[50];
	const char* s = format;
	unsigned int int_count = 0;
	unsigned int float_count = 0;
	//unsigned int long_double_count = 0;
	unsigned int i = 0;
	unsigned int mode = 0;
	unsigned int alt_form = 0;
	unsigned int zero_padding = 0;
	unsigned int signage = 0;
	unsigned int length = 0;
	unsigned int precision = 0;
	while (s[i] != 0) {
		if (mode == 1 || mode == 2) {
			if (s[i] == '%') {
				mode = 0;
				write(fd, "%", 1);
				s += i + 1;
				i = -1;
			} else if (s[i] == '.') {
				alt_form |= 4;
			} else if (s[i] == '-') {
				zero_padding |= 2;
			} else if (s[i] == '+') {
				signage |= 2;
			} else if (s[i] == ' ') {
				signage |= 1;
			} else if (s[i] == '0') {
				if (alt_form & 2) {
					if (alt_form & 4) {
						precision *= 10;
					} else {
						length *= 10;
					}
				} else {
					zero_padding |= 1;
				}
			} else if (s[i] > '0' && s[i] <= '9') {
				alt_form |= 2;
				if (alt_form & 4) {
					precision *= 10;
					precision += s[i] - '0';
				} else {
					length *= 10;
					length += s[i] - '0';
				}
			} else if (s[i] == '#') {
				alt_form |= 1;
				alt_form &= ~2;
			} else if (s[i] == 'L') {
				mode = 2;
				alt_form &= ~2;
			} else if (s[i] == 's') {
				char* pstr;
				if (int_count < 4) {
					pstr = (char*)(*(gen_regs + int_count));
				} else {
					pstr = (char*)stack_params;
					stack_params += sizeof(char*);
				}
				print(pstr);
				s += i + 1;
				i = -1;
				int_count++;
				mode = 0;
			} else if (s[i] == 'd' || s[i] == 'i' || s[i] == 'o' || s[i] == 'u' || s[i] == 'x' || s[i] == 'X' || s[i] == 'c' || s[i] == 'p' || s[i] == 'n' || s[i] == 'm') {
				signed long int tmp_int;
				if (int_count < 4) {
					tmp_int = *(gen_regs + int_count);
				} else {
					tmp_int = *((signed long int*)stack_params);
					stack_params += sizeof(signed long int);
				}
				itoa(tmp_int, buff, 50, 10, 0);
				write(fd, buff, strlen(buff));
				s += i + 1;
				i = -1;
				int_count++;
				mode = 0;
			} else if (s[i] == 'e' || s[i] == 'E' || s[i] == 'f' || s[i] == 'F' || s[i] == 'g' || s[i] == 'G' || s[i] == 'a' || s[i] == 'A') {
				long double tmp_ldouble;
				
				if (mode == 2) {
					// Long Double
					if (float_count < vector_regs_used && float_count < 8) {
						tmp_ldouble = *(vec_regs + float_count);
					} else {
						tmp_ldouble = *((long double*)stack_params);
						stack_params += sizeof(long double);
					}
					//itoa(ftoi(tmp_ldouble), buff, 100, 10, 0);
				} else {
					// Double or Float
					double tmp_double;
					if (float_count < vector_regs_used && float_count < 8) {
						//dprintf(1, "TraceA i: (%d)\n", float_count);
						tmp_double = *((double*)(vec_regs + float_count));
					} else {
						tmp_double = *((double*)stack_params);
						stack_params += sizeof(long double);
					}
					tmp_ldouble = tmp_double;
					//itoa(ftoi(tmp_double), buff, 100, 10, 0);
				}
				
				
				if (tmp_ldouble < 0.0) {
					tmp_ldouble = -tmp_ldouble;
					write(1, "-", 1);
				}
				
				unsigned int k;
				k = 1;
				while (tmp_ldouble >= 1000000000.0) {
					tmp_ldouble /= 1000000000.0;
					k++;
				}
				unsigned int v;
				unsigned int j;
				j = 0;
				while (j < k) {
					v = tmp_ldouble;
					tmp_ldouble -= v;
					if (j == 0) {
						itoa(v, buff, 50, 10, 0);
					} else {
						itoa(v, buff, 50, 10, 9);
					}
					write(1, buff, strlen(buff));
					tmp_ldouble *= 1000000000.0;
					j++;
				}
				
				// Precision
				//tmp_ldouble *= 1000000000.0;
				v = tmp_ldouble;
				//tmp_ldouble -= v;
				itoa(v, buff, 50, 10, 9);
				write(1, ".", 1);
				write(1, buff, strlen(buff) - 3);
				
				//char* testing_str = "[DOUBLE_VALUE_HERE]";
				//itoa(ftoi(2000000000.0), buff, 100, 10, 0);
				//write(fd, testing_str, strlen(testing_str));
				//write(fd, buff, strlen(buff));
				s += i + 1;
				i = -1;
				float_count++;
				mode = 0;
			}
		} else {
			if (s[i] == '%') {
				write(fd, s, i);
				s += i + 1;
				i = -1;
				mode = 1;
				alt_form = 0;
				zero_padding = 0;
				signage = 0;
				length = 0;
				precision = 0;
			}
		}
		i++;
	}
	if (i > 0) {
		write(fd, s, i);
	}
	return 0;
}
*/



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
			for (i = 0; i < (size_t)set_min_width && i < buf_len; i++) {
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
	/*
	int debug = 0;
	uintRL_t debug_dest = (uintRL_t)dest;
	if (debug_dest == 0x80400000) {
		debug = 1;
	}
	*/
	
	const unsigned char* ptr_src = src;
	unsigned char* ptr_dest = dest;
	unsigned char* dest_end = dest + n;
	
	/*
	src and dest need to be XLEN aligned for this to work
	
	uintRL_t* dest_end_lower   = (void*)(((uintRL_t)dest_end) & ~(sizeof(uintRL_t) - 1));
	uintRL_t* dest_start_upper = (void*)(((uintRL_t)dest)     & ~(sizeof(uintRL_t) - 1));
	if (((uintRL_t)dest) & (sizeof(uintRL_t) - 1)) {
		dest_start_upper++;
	}
	
	if (dest_start_upper < dest_end_lower) {
		while (ptr_dest < (unsigned char*)dest_start_upper) {
			*ptr_dest = *ptr_src;
			ptr_src++;
			ptr_dest++;
		}
		uintRL_t* ptr_RL_src = (void*)ptr_src;
		while (dest_start_upper < dest_end_lower) {
			*dest_start_upper = *ptr_RL_src;
			ptr_RL_src++;
			dest_start_upper++;
		}
		ptr_src  = (void*)ptr_RL_src;
		ptr_dest = (void*)dest_start_upper;
	}
	*/
	
	/*
	uintRL_t debug_counter;
	debug_counter = 0;
	debug_dest = 0;
	*/
	while (ptr_dest < dest_end) {
		/*
		if (debug) {
			if (debug_counter == 0x1000) {
				DEBUG_print("Trace: 0x");
				char buf[20];
				itoa(debug_dest, buf, 20, -16, -8);
				DEBUG_print(buf);
				DEBUG_print("\n");
				debug_counter = 0;
			}
			debug_counter++;
			debug_dest++;
		}
		*/
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
