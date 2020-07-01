
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdbool.h>

#include "typedefs.h"
#include "sizes.h"
#include "char32utils.h"

double
_Py_dg_strtod_modified(const char32_t *s00, char32_t **se, int *error,
                       char32_t decimal, char32_t sci, bool skip_trailing);

/*
 *  `item` must be the nul-terminated string that is to be
 *  converted to a double.
 *
 *  To be successful, to_double() must use *all* the characters
 *  in `item`.  E.g. "1.q25" will fail.  Leading and trailing 
 *  spaces are allowed.
 *
 *  `sci` is the scientific notation exponent character, usually
 *  either 'E' or 'D'.  Case is ignored.
 *
 *  `decimal` is the decimal point character, usually either
 *  '.' or ','.
 *
 */

bool to_double(char32_t *item, double *p_value, char32_t sci, char32_t decimal)
{
    char32_t *p_end;
    int error;

    *p_value = _Py_dg_strtod_modified(item, &p_end, &error, decimal, sci, true);

    return (error == 0) && (!*p_end);
}


bool to_complex(char32_t *item, double *p_real, double *p_imag,
                char32_t sci, char32_t decimal)
{
    char32_t *p_end;
    int error;

    *p_real = _Py_dg_strtod_modified(item, &p_end, &error, decimal, sci, false);
    if (*p_end == '\0') {
        // No imaginary part in the string (e.g. "3.5")
        *p_imag = 0.0;
        return error == 0;
    }
    if (*p_end == 'i' || *p_end == 'j') {
        // Pure imaginary part only (e.g "1.5j")
        *p_imag = *p_real;
        *p_real = 0.0;
        ++p_end;
    }
    else {
        if (*p_end == '+') {
            ++p_end;
        }
        *p_imag = _Py_dg_strtod_modified(p_end, &p_end, &error, decimal, sci, false);
        if (error || ((*p_end != 'i') && (*p_end != 'j'))) {
            return false;
        }
        ++p_end;
    }
    while(*p_end == ' ') {
        ++p_end;
    }
    return *p_end == '\0';
}


bool to_longlong(char32_t *item, long long *p_value)
{
    char32_t *p_end;

    // Try integer conversion.
    *p_value = strtoll32(item, &p_end);

    // Allow trailing spaces.
    while (isspace(*p_end)) ++p_end;

    return (errno == 0) && (!*p_end);
}
