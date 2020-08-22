#include "./string.h"

void itoa(register signed int number, register char* buf, register signed int buf_len, register signed int base, register signed int set_width) {
	register signed long int num;
	num = number;
	if (base < 0) {
		base = -base;
		if(number < 0) {
			num = -num;
		}
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
