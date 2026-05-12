#include <stdint.h>
#include <time.h>

#define TIMER_HZ 19200000

static volatile uint64_t cycle_counter_init_value = 0;

static inline uint64_t read_cntpct_el0(void) {
    uint64_t val;
    asm volatile("mrs %0, cntpct_el0" : "=r" (val));
    return val;
}

void _clock_init(void) {
    cycle_counter_init_value = read_cntpct_el0();
}

clock_t clock(void) {
    uint64_t current = read_cntpct_el0();
    uint64_t ticks = current - cycle_counter_init_value;
    return (clock_t)(ticks * CLOCKS_PER_SEC / TIMER_HZ);
}
