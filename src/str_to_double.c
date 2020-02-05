//
// str_to_double.c
//

#include <stdio.h>

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <float.h>


//#define isdigit(c) (((c) >= '0') && ((c) <= '9'))
//#define isspace(c) (((c) == ' ') || ((c) == '\t'))

// pow10table_size is 309.
extern double pow10table_size;
// pow10table[k] is 10**k, for k = 0, 1, ..., 308.
extern double pow10table[];


double str_to_double(const char *str, char **end, int *error,
                     char decimal, char sci, int skip_trailing)
{
    double value;
    int negative, negative_exp;
    char *p = (char *) str;
    int n;
    int has_leading_zero;
    int num_digits;
    int num_decimals;

    *error = 0;
    sci = toupper(sci);

    value = 0.0;
    num_digits = 0;
    num_decimals = 0;

    // Skip initial whitespace.
    while (isspace(*p)) {
        ++p;
    }

    // Check for a sign character.
    negative = *p == '-';
    if (negative || (*p == '+')) {
        ++p;
    }

    // Skip leading zeros.
    has_leading_zero = 0;
    if (*p == '0') {
        ++p;
        has_leading_zero = 1;
    }
    while (*p == '0') {
        ++p;
    }
    if (has_leading_zero) {
        // Keep one leading zero, but don't count it.
        --p;
        --num_digits;
    }

    // Process digits.
    while (isdigit(*p)) {
        if (num_digits < 19) {
            value = value * 10.0 + (double) (*p - '0');
            ++num_digits;
        }
        ++p;
    }

    // Process decimal part.
    if (*p == decimal) {
        //double frac = 0.0;
        ++p;
        while (isdigit(*p)) {
            if (num_digits < 19) {
                value = 10*value + (double) (*p - '0');
                ++num_decimals;
                ++num_digits;
            }
            ++p;
        }
    }

    if (num_digits == 0 && !has_leading_zero) {
        // If num_digits is 0 and there was no leading zero, it means
        // we didn't find a number.
        // XXX Why is the error ERANGE?
        *error = ERANGE;
        if (end) {
            *end = p;
        }
        return 0.0;
    }

    // Set the sign.
    if (negative) {
        value = -value;
    }

    // Check for an exponent.
    n = 0;
    negative_exp = 0;
    if (toupper(*p) == sci) {
        ++p;

        // Get the sign.
        negative_exp = *p == '-';
        if (negative_exp || (*p == '+')) {
            ++p;
        }

        // Skip leading zeros.
        while (*p == '0') {
            ++p;
        }

        n = 0;
        while (isdigit(*p)) {
            n = n * 10 + (int) (*p - '0');
            ++p;
        }
    }

    if (negative_exp) {
        n = -n;
    }

    // XXX The following adjustments need more thought, and unit tests.
    n -= num_decimals;
    if (n < -308) {
        int m;
        n += num_digits-1;
        m = num_digits-1;
        if (n < -308) {
            n += 16;
            m += 16;
        }
        value /= pow10table[m];
    }

    if (n < 0) {
        if (-n >= pow10table_size) {
            if (negative) {
                value = -0.0;
            }
            else {
                value = 0.0;
            }
        }
        else {
            value /= pow10table[-n];
        }
    }
    else {
        if (n >= pow10table_size) {
            if (negative) {
                value = -INFINITY;
            }
            else {
                value = INFINITY;
            }
        }
        else {
            value *= pow10table[n];
        }
    }

    if (skip_trailing) {
        // Skip trailing whitespace
        while (isspace(*p)) {
            ++p;
        }
    }

    if (end) {
        *end = p;
    }

    return value;
}
