#ifndef STR_TO_H
#define STR_TO_H

#include <stdint.h>

#include "typedefs.h"


#define ERROR_OK             0
#define ERROR_NO_DIGITS      1
#define ERROR_OVERFLOW       2
#define ERROR_INVALID_CHARS  3
#define ERROR_MINUS_SIGN     4

int64_t str_to_int64(const char32_t *p_item, int64_t int_min, int64_t int_max, int *error);
uint64_t str_to_uint64(const char32_t *p_item, uint64_t uint_max, int *error);

#endif
