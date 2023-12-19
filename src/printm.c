#include "printm.h"
#include <string.h>
#include <thread_locking.h>
#include <drivers/uart.h>
#include <inc/memmap.h>
#include <inc/var_args.h>

void DEBUG_print(char* str) {
	uart_write((unsigned char*)str, UART0_BASE, strlen(str));
	return;
}

static void _printm(const char* restrict buffer, size_t length) {
	uart_write((unsigned char*)buffer, UART0_BASE, length);
	return;
}

static size_t _itoa(unsigned long long number, char* restrict buf, size_t buf_len, signed char base) {
	unsigned char hex_offset = 'a' - 10;
	if (base < 0) {
		hex_offset = 'A' - 10;
		base = -base;
	}
	
	size_t i = 0;
	while (number > 0 && i < buf_len) {
		unsigned char digit;
		
		digit  = number % base;
		number = number / base;
		
		if (digit >= 10) {
			buf[i] = digit + hex_offset;
		} else {
			buf[i] = digit + '0';
		}
		
		i++;
	}
	
	if (buf_len > 0 && i == 0) {
		buf[0] = '0';
		i++;
	}
	
	size_t j = i;
	size_t k = 0;
	while (j > 0) {
		j--;
		if (j <= k) {
			break;
		}
		
		unsigned char tmp_digit;
		tmp_digit = buf[k];
		buf[k] = buf[j];
		buf[j] = tmp_digit;
		
		k++;
	}
	
	return i;
}

#define flags_special 0x0001

#define flags_altform 0x0002
#define flags_zeropad 0x0004
#define flags_leftadj 0x0008
#define flags_spacepd 0x0010
#define flags_asigned 0x0020
#define flags_decimal 0x0040 // Not implemented
#define flags_intdecl 0x0080 // Not implemented

#define flags_lmod_hh 0x0100
#define flags_lmod_h_ 0x0200
#define flags_lmod_l_ 0x0400
#define flags_lmod_ll 0x0800
#define flags_lmod_j_ 0x1000 // Not implemented
#define flags_lmod_z_ 0x2000 // Not implemented
#define flags_lmod_t_ 0x4000 // Not implemented

#define flags_fdwidth 0x8000

ksemaphore_t printm_lock = 0;

void printm(const char* restrict format, ...) {
	va_list ap;
	va_start(ap, format);
	
	ksem_wait(&printm_lock);
	
	char buf[0x40];
	unsigned int i = 0;
	unsigned int j = 0;
	unsigned int flags = 0;
	unsigned int fdwidth = 0;
	while (format[i]) {
		if ((format[i] != '%') && (flags == 0)) {
			buf[j] = format[i];
			j++;
			if (j == 0x40) {
				// Buffer is full.  Flush it.
				_printm(buf, j);
				j = 0;
			}
			goto next_format_character;
		}
		
		if (flags & flags_special) {
			// Field Width Modifiers
			if (((format[i] == '0') && (flags & flags_fdwidth)) || ((format[i] >= '1') && (format[i] <= '9'))) {
				fdwidth *= 10;
				fdwidth += format[i] - '0';
				flags |= flags_fdwidth;
				goto next_format_character;
			}
			flags &= ~flags_fdwidth;
			
			// Flag Modifiers
			if (format[i] == '#') {
				flags |= flags_altform;
				goto next_format_character;
			}
			if (format[i] == '0') {
				flags |= flags_zeropad;
				goto next_format_character;
			}
			if (format[i] == '-') {
				flags |= flags_leftadj;
				goto next_format_character;
			}
			if (format[i] == ' ') {
				flags |= flags_spacepd;
				goto next_format_character;
			}
			if (format[i] == '+') {
				flags |= flags_asigned;
				goto next_format_character;
			}
			
			// Length/Type Modifiers
			if (format[i] == 'h') {
				if (flags & flags_lmod_h_) {
					flags |= flags_lmod_hh;
				}
				flags |= flags_lmod_h_;
				goto next_format_character;
			}
			if (format[i] == 'l') {
				if (flags & flags_lmod_l_) {
					flags |= flags_lmod_ll;
				}
				flags |= flags_lmod_l_;
				goto next_format_character;
			}
			
			// Conversion Specifiers
			if ((format[i] == 'd') || (format[i] == 'i')) {
				// Signed Decimal
				
				signed long long num;
				if        (flags & flags_lmod_hh) {
					signed char tmp_num;
					tmp_num = (signed char)va_arg(ap, signed int);
					num = tmp_num;
				} else if (flags & flags_lmod_ll) {
					signed long long tmp_num;
					tmp_num = va_arg(ap, signed long long);
					num = tmp_num;
				} else if (flags & flags_lmod_h_) {
					signed short tmp_num;
					tmp_num = (signed short)va_arg(ap, signed int);
					num = tmp_num;
				} else if (flags & flags_lmod_l_) {
					signed long tmp_num;
					tmp_num = va_arg(ap, signed long);
					num = tmp_num;
				} else {
					signed int tmp_num;
					tmp_num = va_arg(ap, signed int);
					num = tmp_num;
				}
				
				char* prefix = "";
				size_t full_number_length = 0;
				unsigned long long number;
				if (num < 0) {
					number = -num;
					prefix = "-";
					full_number_length = 1;
				} else {
					number = num;
					if        (flags & flags_asigned) {
						prefix = "+";
						full_number_length = 1;
					} else if (flags & flags_spacepd) {
						prefix = " ";
						full_number_length = 1;
					}
				}
				
				size_t number_length;
				number_length = _itoa(number, buf, 0x40,  10);
				
				char* pad = " ";
				if (flags & flags_leftadj) {
					_printm(prefix, full_number_length);
					prefix = 0;
					_printm(buf, number_length);
				} else if (flags & flags_zeropad) {
					_printm(prefix, full_number_length);
					prefix = 0;
					pad = "0";
				}
				if (full_number_length == 0) {
					prefix = 0;
				}
				
				full_number_length += number_length;
				while (full_number_length < fdwidth) {
					_printm(pad, 1);
					full_number_length++;
				}
				
				if (prefix) {
					_printm(prefix, 1);
				}
				if (!(flags & flags_leftadj)) {
					_printm(buf, number_length);
				}
				
				goto end_special;
			}
			if  (format[i] == 'u') {
				// Unsigned Decimal
				
				unsigned long long num;
				if        (flags & flags_lmod_hh) {
					unsigned char tmp_num;
					tmp_num = (unsigned char)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_ll) {
					unsigned long long tmp_num;
					tmp_num = va_arg(ap, unsigned long long);
					num = tmp_num;
				} else if (flags & flags_lmod_h_) {
					unsigned short tmp_num;
					tmp_num = (unsigned short)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_l_) {
					unsigned long tmp_num;
					tmp_num = va_arg(ap, unsigned long);
					num = tmp_num;
				} else {
					unsigned int tmp_num;
					tmp_num = va_arg(ap, unsigned int);
					num = tmp_num;
				}
				
				unsigned long long number;
				number = num;
				
				size_t full_number_length = 0;
				size_t number_length;
				number_length = _itoa(number, buf, 0x40,  10);
				full_number_length += number_length;
				
				char* pad = " ";
				if (flags & flags_leftadj) {
					_printm(buf, number_length);
				} else if (flags & flags_zeropad) {
					pad = "0";
				}
				
				while (full_number_length < fdwidth) {
					_printm(pad, 1);
					full_number_length++;
				}
				
				if (!(flags & flags_leftadj)) {
					_printm(buf, number_length);
				}
				
				goto end_special;
			}
			if  (format[i] == 'o') {
				// Unsigned Octal
				
				unsigned long long num;
				if        (flags & flags_lmod_hh) {
					unsigned char tmp_num;
					tmp_num = (unsigned char)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_ll) {
					unsigned long long tmp_num;
					tmp_num = va_arg(ap, unsigned long long);
					num = tmp_num;
				} else if (flags & flags_lmod_h_) {
					unsigned short tmp_num;
					tmp_num = (unsigned short)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_l_) {
					unsigned long tmp_num;
					tmp_num = va_arg(ap, unsigned long);
					num = tmp_num;
				} else {
					unsigned int tmp_num;
					tmp_num = va_arg(ap, unsigned int);
					num = tmp_num;
				}
				
				unsigned long long number;
				number = num;
				
				size_t full_number_length = 0;
				size_t number_length;
				number_length = _itoa(number, buf, 0x40,   8);
				full_number_length += number_length;
				
				char* pad = " ";
				if (flags & flags_leftadj) {
					if (flags & flags_altform) {
						_printm("0", 1);
					}
					_printm(buf, number_length);
					flags |= flags_zeropad;
				} else if (flags & flags_zeropad) {
					if (flags & flags_altform) {
						_printm("0", 1);
					}
					pad = "0";
				}
				if (flags & flags_altform) {
					full_number_length += 1;
				}
				
				while (full_number_length < fdwidth) {
					_printm(pad, 1);
					full_number_length++;
				}
				
				if (!(flags & flags_zeropad)) {
					if (flags & flags_altform) {
						_printm("0", 1);
					}
				}
				
				if (!(flags & flags_leftadj)) {
					_printm(buf, number_length);
				}
				
				goto end_special;
			}
			if (format[i] == 'x' || format[i] == 'p') {
				// Unsigned Lower-Case Hexadecimal
				
				unsigned long long num;
				if        (format[i] == 'p') {
					uintRL_t tmp_num;
					tmp_num = (uintRL_t)(va_arg(ap, void*));
					num = (unsigned long long)tmp_num;
				} else if (flags & flags_lmod_hh) {
					unsigned char tmp_num;
					tmp_num = (unsigned char)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_ll) {
					unsigned long long tmp_num;
					tmp_num = va_arg(ap, unsigned long long);
					num = tmp_num;
				} else if (flags & flags_lmod_h_) {
					unsigned short tmp_num;
					tmp_num = (unsigned short)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_l_) {
					unsigned long tmp_num;
					tmp_num = va_arg(ap, unsigned long);
					num = tmp_num;
				} else {
					unsigned int tmp_num;
					tmp_num = va_arg(ap, unsigned int);
					num = tmp_num;
				}
				
				unsigned long long number;
				number = num;
				
				size_t full_number_length = 0;
				size_t number_length;
				number_length = _itoa(number, buf, 0x40,  16);
				full_number_length += number_length;
				
				char* pad = " ";
				if (flags & flags_leftadj) {
					if (flags & flags_altform) {
						_printm("0x", 2);
					}
					_printm(buf, number_length);
					flags |= flags_zeropad;
				} else if (flags & flags_zeropad) {
					if (flags & flags_altform) {
						_printm("0x", 2);
					}
					pad = "0";
				}
				if (flags & flags_altform) {
					full_number_length += 2;
				}
				
				while (full_number_length < fdwidth) {
					_printm(pad, 1);
					full_number_length++;
				}
				
				if (!(flags & flags_zeropad)) {
					if (flags & flags_altform) {
						_printm("0x", 2);
					}
				}
				
				if (!(flags & flags_leftadj)) {
					_printm(buf, number_length);
				}
				
				goto end_special;
			}
			if  (format[i] == 'X') {
				// Unsigned Upper-Case Hexadecimal
				
				unsigned long long num;
				if        (flags & flags_lmod_hh) {
					unsigned char tmp_num;
					tmp_num = (unsigned char)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_ll) {
					unsigned long long tmp_num;
					tmp_num = va_arg(ap, unsigned long long);
					num = tmp_num;
				} else if (flags & flags_lmod_h_) {
					unsigned short tmp_num;
					tmp_num = (unsigned short)va_arg(ap, unsigned int);
					num = tmp_num;
				} else if (flags & flags_lmod_l_) {
					unsigned long tmp_num;
					tmp_num = va_arg(ap, unsigned long);
					num = tmp_num;
				} else {
					unsigned int tmp_num;
					tmp_num = va_arg(ap, unsigned int);
					num = tmp_num;
				}
				
				unsigned long long number;
				number = num;
				
				size_t full_number_length = 0;
				size_t number_length;
				number_length = _itoa(number, buf, 0x40, -16);
				full_number_length += number_length;
				
				char* pad = " ";
				if (flags & flags_leftadj) {
					if (flags & flags_altform) {
						_printm("0X", 2);
					}
					_printm(buf, number_length);
					flags |= flags_zeropad;
				} else if (flags & flags_zeropad) {
					if (flags & flags_altform) {
						_printm("0X", 2);
					}
					pad = "0";
				}
				if (flags & flags_altform) {
					full_number_length += 2;
				}
				
				while (full_number_length < fdwidth) {
					_printm(pad, 1);
					full_number_length++;
				}
				
				if (!(flags & flags_zeropad)) {
					if (flags & flags_altform) {
						_printm("0X", 2);
					}
				}
				
				if (!(flags & flags_leftadj)) {
					_printm(buf, number_length);
				}
				
				goto end_special;
			}
			if  (format[i] == 'c') {
				// Character
				
				unsigned char num;
				{
					unsigned char tmp_num;
					tmp_num = (unsigned char)va_arg(ap, unsigned int);
					num = tmp_num;
				}
				
				buf[0] = num;
				_printm(buf, 1);
				
				goto end_special;
			}
			if  (format[i] == 's') {
				// C-String
				
				void* num;
				{
					void* tmp_num;
					tmp_num = va_arg(ap, void*);
					num = tmp_num;
				}
				
				void* number;
				number = num;
				
				size_t str_length;
				
				if (fdwidth > 0) {
					str_length = strnlen(number, fdwidth);
				} else {
					str_length = strlen(number);
				}
				
				_printm(number, str_length);
				
				goto end_special;
			}
			if  (format[i] == '%') {
				// Delimited %
				_printm("%", 1);
				goto end_special;
			}
			
			// Not supported, but known in standards
			if ((format[i] == 'e') || (format[i] == 'E') || \
			    (format[i] == 'f') || (format[i] == 'F') || \
			    (format[i] == 'g') || (format[i] == 'G') || \
			    (format[i] == 'a') || (format[i] == 'A') || \
			    (format[i] == 'C') || \
			    (format[i] == 'S') || \
			    (format[i] == 'n') || \
			    (format[i] == 'M')) {
				end_special:
				flags = 0;
				j = 0;
			}
			
		} else {
			// Enter Special Section (Enter %)
			
			// Flush the buffer
			_printm(buf, j);
			j = 0;
			
			// Initialize variables
			fdwidth = 0;
			
			// Set the Special flag
			flags = flags_special;
		}
		
		next_format_character:
		i++;
	}
	if (j > 0) {
		_printm(buf, j);
	}
	
	ksem_post(&printm_lock);
	
	va_end(ap);
	return;
}
