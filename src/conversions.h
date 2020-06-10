#ifndef CONVERSIONS_H
#define CONVERSIONS_H

#include <stdbool.h>

#include "typedefs.h"


bool to_double(char32_t *item, double *p_value, char32_t sci, char32_t decimal);
bool to_complex(char32_t *item, double *p_real, double *p_imag, char32_t sci, char32_t decimal);
bool to_longlong(char32_t *item, long long *p_value);

#endif
