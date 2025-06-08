#ifndef TYPES_H
 #define TYPES_H
#include <stdbool.h>
#include <stdint.h>

#define MIN_U64TOA_SIZE (size_t)21

bool is_string_number(char *);
char *u64toa(char *, size_t, uint64_t);

#endif
