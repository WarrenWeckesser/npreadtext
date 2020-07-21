#ifndef _TYPE_INFERENCE_H_
#define _TYPE_INFERENCE_H_

#include "typedefs.h"

char classify_type(char32_t *field, char32_t decimal, char32_t sci, char32_t imaginary_unit,
                   int64_t *i, uint64_t *u,
                   char prev_type);
char type_for_integer_range(int64_t imin, uint64_t umax);

#endif
