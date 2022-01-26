#ifndef _insertion_var_args_h
#define _insertion_var_args_h

typedef __builtin_va_list va_list;

#define va_start(p1, p2) __builtin_va_start(p1, p2)
#define va_arg(p1, p2) __builtin_va_arg(p1, p2)
#define va_copy(p1, p2) __builtin_va_copy(p1, p2)
#define va_end(p1) __builtin_va_end(p1)

#endif
