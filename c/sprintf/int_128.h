#ifndef INT128_H
#define INT128_H

#include <stdint.h>

typedef struct
{
	uint64_t high;
	uint64_t low;
} int128_t;

int128_t parse_number(const char *str);
void number_to_string(int128_t value, int base, char *buffer, int uppercase);

#endif
