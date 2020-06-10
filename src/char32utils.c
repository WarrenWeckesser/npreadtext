
#include "char32utils.h"


size_t strlen32(char32_t *s)
{
    size_t count = 0;
    while (*s) {
        ++count;
        ++s;
    }
    return count;
}


long long strtoll32(char32_t *s, char32_t **p)
{
    long long result = 0;
    int sign = 1;

    while (isspace(*s)) {
        ++s;
    }
    if (*s == '-') {
        sign = -1;
        ++s;
    }
    else if (*s == '+') {
        ++s;
    }
    while (isdigit(*s)) {
        result = 10*result + (*s - '0');
        ++s;
    }
    *p = s;
    result *= sign;
    return result;
}