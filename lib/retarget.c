//--------------------------------------------------------------------
// Description:
//    Library re-target file for C-based test code
//--------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/stat.h>
#include <unistd.h>
#include "output.h"

#define MAX_BUFFER 512
#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')

extern char __bss_start__[], __bss_end__[];
extern void __libc_init_array(void);
extern void __libc_fini_array(void);

void initialise_monitor_handles(void) { }

int _close(int fh) { }
int _gettimeofday(struct timeval* tv, void* tz) { }
int _isatty(int fh) { }
int _lseek(int fh, off_t offset, int whence) { }
int _open(const char *path, int oflag, /* int mode */ ...) { return 1; }
int _read(int fh, unsigned char *buffer, int count) { }
int _rename(const char *old, const char *new ) { }
clock_t _times(struct tms *buf) {
    clock_t t = clock();
    if (buf) {
        buf->tms_utime = t;
        buf->tms_stime = 0;
        buf->tms_cutime = 0;
        buf->tms_cstime = 0;
    }
    return t;
}
int _unlink(const char *name) { }
int _kill(int pid, int sig) { }
int _fstat(int fildes, struct stat *buf) { }

// Redirect output (from printf etc) to the tube.
// This redirects all streams to the tube.
int _write(int fh, char *buf, int count)
{
    int i = 0;

    while (i < count)
        output_char(buf[i++]);

    return count;
}

// Support functions for number conversions
void reverse(char* str, int length) {
    int start = 0;
    int end = length - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}

char* ltoa(long value, char* str, int base) {
    int i = 0;
    bool negative = false;

    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if (value < 0 && base == 10) {
        negative = true;
        value = -value;
    }

    while (value != 0) {
        int rem = value % base;
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
        value = value / base;
    }

    if (negative)
        str[i++] = '-';

    str[i] = '\0';
    reverse(str, i);
    return str;
}

// Convert float to string with specified precision
void ftoa(double num, char *str, int precision) {
    int whole = (int)num;
    ltoa(whole, str, 10);
    int len = strlen(str);

    if (precision > 0) {
        str[len] = '.';
        double frac = num - whole;
        if (frac < 0) frac = -frac;

        for (int i = 0; i < precision; i++) {
            frac *= 10;
            int digit = (int)frac;
            str[len + 1 + i] = '0' + digit;
            frac -= digit;
        }
        str[len + 1 + precision] = '\0';
    }
}


int printf(const char* format, ...) {
    char buffer[MAX_BUFFER];
    int buf_index = 0;

    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0' && buf_index < MAX_BUFFER - 2; i++) {
        if (format[i] != '%') {
            buffer[buf_index++] = format[i];
            continue;
        }

        i++;  // Move past '%'

        // Skip flags
        bool left_align = false;
        while (format[i] == '-' || format[i] == '0' ||
               format[i] == '+' || format[i] == ' ') {
            if (format[i] == '-') left_align = true;
            i++;
        }

        // Get width
        int width = 0;
        while (IS_DIGIT(format[i])) {
            width = width * 10 + (format[i] - '0');
            i++;
        }

        // Get precision
        int precision = -1;
        if (format[i] == '.') {
            precision = 0;
            i++;
            while (IS_DIGIT(format[i])) {
                precision = precision * 10 + (format[i] - '0');
                i++;
            }
        }

        // Check for long modifier
        bool is_long = false;
        if (format[i] == 'l') {
            is_long = true;
            i++;
        }

        // Handle conversion
        char temp[32] = {0};
        int temp_len = 0;

        switch (format[i]) {
            case 'd': {
                long value;
                if (is_long) {
                    value = va_arg(args, long);
                } else {
                    value = va_arg(args, int);
                }

                if (value < 0) {
                    temp[temp_len++] = '-';
                    value = -value;
                }

                if (value == 0) {
                    temp[temp_len++] = '0';
                } else {
                    char num[20];
                    int num_len = 0;
                    while (value > 0) {
                        num[num_len++] = '0' + (value % 10);
                        value /= 10;
                    }
                    while (num_len > 0) {
                        temp[temp_len++] = num[--num_len];
                    }
                }
                break;
            }

            case 's': {
                char* str = va_arg(args, char*);
                // Write directly to buffer (skip temp to avoid overflow)
                int padding_s = width - (int)strlen(str);
                if (!left_align && padding_s > 0) {
                    while (padding_s-- > 0 && buf_index < MAX_BUFFER - 2)
                        buffer[buf_index++] = ' ';
                }
                while (*str && buf_index < MAX_BUFFER - 2) {
                    buffer[buf_index++] = *str++;
                }
                if (left_align && padding_s > 0) {
                    while (padding_s-- > 0 && buf_index < MAX_BUFFER - 2)
                        buffer[buf_index++] = ' ';
                }
                continue; // skip the common temp→buffer copy below
            }

            case 'c': {
                char c = (char)va_arg(args, int);
                temp[temp_len++] = c;
                break;
            }

            case 'x': {
                unsigned long value;
                if (is_long) {
                    value = va_arg(args, unsigned long);
                } else {
                    value = va_arg(args, unsigned int);
                }

                if (value == 0) {
                    temp[temp_len++] = '0';
                } else {
                    char num[16];
                    int num_len = 0;
                    while (value > 0) {
                        int digit = value % 16;
                        num[num_len++] = digit < 10 ? '0' + digit : 'a' + digit - 10;
                        value /= 16;
                    }
                    while (num_len > 0) {
                        temp[temp_len++] = num[--num_len];
                    }
                }
                break;
            }

            case 'u': {
                unsigned long value;
                if (is_long) {
                    value = va_arg(args, unsigned long);
                } else {
                    value = va_arg(args, unsigned int);
                }

                if (value == 0) {
                    temp[temp_len++] = '0';
                } else {
                    char num[20];
                    int num_len = 0;
                    while (value > 0) {
                        num[num_len++] = '0' + (value % 10);
                        value /= 10;
                    }
                    while (num_len > 0) {
                        temp[temp_len++] = num[--num_len];
                    }
                }
                break;
            }

case 'e': {
    double value = va_arg(args, double);
    if (precision < 0) precision = 6;

    if (value < 0) {
        temp[temp_len++] = '-';
        value = -value;
    }

    // Get exponent
    int exp = 0;
    if (value != 0.0) {
        while (value >= 10.0) { value /= 10.0; exp++; }
        while (value < 1.0) { value *= 10.0; exp--; }
    }

    // First digit and decimal point
    temp[temp_len++] = '0' + (int)value;
    temp[temp_len++] = '.';
    value -= (int)value;

    // Decimal digits
    for (int i = 0; i < precision; i++) {
        value *= 10.0;
        temp[temp_len++] = '0' + (int)value;
        value -= (int)value;
    }

    // Exponent
    temp[temp_len++] = 'e';
    if (exp < 0) {
        temp[temp_len++] = '-';
        exp = -exp;
    } else {
        temp[temp_len++] = '+';
    }

    // Variable exponent digits
    char expstr[10];
    int explen = 0;
    do {
        expstr[explen++] = '0' + (exp % 10);
        exp /= 10;
    } while (exp > 0);

    // Pad with zeros to at least 2 digits
    while (explen < 2) expstr[explen++] = '0';

    // Copy exponent digits in reverse
    while (explen > 0) {
        temp[temp_len++] = expstr[--explen];
    }
    break;
}

            case 'f': {
                if (precision < 0) precision = 6;
                double value = va_arg(args, double);

                if (value < 0) {
                    temp[temp_len++] = '-';
                    value = -value;
                }

                // Handle rounding based on precision
                double multiplier = 1.0;
                for (int i = 0; i < precision; i++) {
                    multiplier *= 10.0;
                }
                value = value * multiplier + 0.5;
                value /= multiplier;

                // Integer part
                long int_part = (long)value;
                if (int_part == 0) {
                    temp[temp_len++] = '0';
                } else {
                    char num[20];
                    int num_len = 0;
                    while (int_part > 0) {
                        num[num_len++] = '0' + (int_part % 10);
                        int_part /= 10;
                    }
                    for (int j = num_len - 1; j >= 0; j--) {
                        temp[temp_len++] = num[j];
                    }
                }

                // Fractional part
                if (precision > 0) {
                    temp[temp_len++] = '.';
                    double frac = value - (long)value;
                    for (int i = 0; i < precision; i++) {
                        frac *= 10;
                        int digit = (int)frac;
                        temp[temp_len++] = '0' + digit;
                        frac -= digit;
                    }
                }
                break;
            }
        }

        // Handle padding
        int padding = width - temp_len;
        if (padding > 0) {
            if (left_align) {
                // Copy string first, then pad
                for (int j = 0; j < temp_len; j++) {
                    buffer[buf_index++] = temp[j];
                }
                while (padding-- > 0) {
                    buffer[buf_index++] = ' ';
                }
            } else {
                // Pad first, then copy string
                while (padding-- > 0) {
                    buffer[buf_index++] = ' ';
                }
                for (int j = 0; j < temp_len; j++) {
                    buffer[buf_index++] = temp[j];
                }
            }
        } else {
            // No padding needed
            for (int j = 0; j < temp_len; j++) {
                buffer[buf_index++] = temp[j];
            }
        }
    }

    va_end(args);

    // Process \n to \r\n
    char final[MAX_BUFFER];
    int final_index = 0;

    for (int i = 0; i < buf_index && final_index < MAX_BUFFER - 2; i++) {
        if (buffer[i] == '\n') {
            final[final_index++] = '\r';
        }
        final[final_index++] = buffer[i];
    }

    return _write(1, final, final_index);
}




// On exit write CTRL-D (EOT character) to the validation tube
// which causes the simulation to terminate
void _exit(int c)
{
    output_char('\x04');  // CTRL-D (EOT)

    // Loop forever until the simulator terminates
    while (1);
}

caddr_t _sbrk_r ( struct _reent *r, int incr )
{
    extern unsigned char bottom_of_heap asm ("heap_base");
    register unsigned char* stack_pointer asm ("sp");

    static unsigned char *heap_end;
    unsigned char        *prev_heap_end;

    if (heap_end == NULL)
        heap_end = &bottom_of_heap;

    prev_heap_end = heap_end;

    if (heap_end + incr > stack_pointer) {
        r->_errno = ENOMEM;
        return (caddr_t) -1;
    }

    heap_end += incr;

    return (caddr_t) prev_heap_end;
}

void init_libc(void)
{
    // Zero the BSS first
    char *start = __bss_start__;
    char *end = __bss_end__;
    while (start < end) {
        *start++ = 0;
    }

    // Initialize stdio
    initialise_monitor_handles();

    atexit(__libc_fini_array);
    __libc_init_array();
}

extern int main(int argc, char **argv);

int _program_start()
{
    // Zero the BSS first
    char *start = __bss_start__;
    char *end = __bss_end__;
    while (start < end) {
        *start++ = 0;
    }

    // Go directly to main
    main(0, NULL);

    // Loop forever
    for (;;) {
        __asm__ volatile ("wfi");
    }
}
