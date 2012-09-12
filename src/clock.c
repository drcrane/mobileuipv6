#include <stdio.h>
#include <string.h>
#include <time.h>

#include "clock.h"

static clock_time_t global_system_ticks = 0;

clock_time_t clock_time() {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, (struct timespec *)&tp);
	return ((tp.tv_sec << 12 & 0xfffff000) | (tp.tv_nsec >> 20 & 0xfff)) - global_system_ticks;
}

unsigned long clock_seconds(void)
{
	return (clock_time() / 4101);
}

void clock_init() {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, (struct timespec *)&tp);
	global_system_ticks = ((tp.tv_sec << 12 & 0xfffff000) | (tp.tv_nsec >> 20 & 0xfff));
}
