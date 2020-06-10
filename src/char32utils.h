#ifndef CHAR32UTILS_H
#define CHAR32UTILS_H

#include <stddef.h>

#include "typedefs.h"

size_t strlen32(char32_t *s);
long long strtoll32(char32_t *s, char32_t **p);

#endif
