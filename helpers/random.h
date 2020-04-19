#ifndef MYY_RANDOM_H
#define MYY_RANDOM_H 1

#include <stdlib.h>
#include <stdint.h>

__attribute__((unused))
static uint8_t myy_random8(void) {
	return (uint8_t) (rand() & UINT8_MAX);
}

#endif

