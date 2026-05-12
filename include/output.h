#ifndef OUTPUT_H
#define OUTPUT_H

#ifdef printf
#undef printf
#endif

int printf(const char *format, ...);

#endif
void output_char(char c);
