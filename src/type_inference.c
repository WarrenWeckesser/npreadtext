#define _XOPEN_SOURCE

#include <stdio.h>
#include <time.h>

#include "str_to.h"
#include "conversions.h"

/*

Some use cases:

Suppose the file contains:
-----------------------------
100,1.2,,,
200,1.4,,,
300,1.8,,,
400,2.0,19,-1,5.0
500,2.5,21,-3,7.5
-----------------------------
Then the last three columns should be classified as
'B', 'b', 'd', respectively, and the data for the
first three rows for these columns should be treated
as missing data.

*/

/*
 *  char classify_type(char *field, char decimal, char sci, int64_t *i, uint64_t *u, char *datetime_fmt)
 *
 *  Try to parse the field, in the following order:
 *      unsigned int  ('Q')
 *      int ('q')
 *      floating point ('d')
 *      complex ('z')
 *      datetime ('U')
 *  If those all fail, the field type is called 's'.
 *
 *  Only strings that match the given datetime format will
 *  be classified as 'U'.
 *
 *  If the classification is 'Q' or 'q', the value
 *  of the integer is stored in *u or *i, resp.
 *
 *  prev_type == '*' means there is no previous sample from this column.
 *
 *  XXX How should a fields of spaces be classified?
 *      What about an empty field, ""?
 *      In isolation, a blank field could be classified '*'.  How should
 *      blanks in a column be used?  It seems that, if the prev_type is '*',
 *      it means we don't what the column is.  If a blank field is encountered
 *      when prev_type != '*', the field should stay classified as prev_type.
 *      When we are using prev_type, the problem we are solving is actually
 *      to classify a column, not just a single field. 
 */

char classify_type(char *field, char decimal, char sci, int64_t *i, uint64_t *u,
                   //char *datetime_fmt,
                   char prev_type)
{
    int error = 0;
    int success;
    double real, imag;
    //struct tm tm;

    switch (prev_type) {
        case '*':
        case 'Q':
        case 'q':
            *u = str_to_uint64(field, UINT64_MAX, &error);
            if (error == 0) {
                return 'Q';
            }
            if (error == ERROR_MINUS_SIGN) {
                *i = str_to_int64(field, INT64_MIN, INT64_MAX, &error);
                if (error == 0) {
                    return 'q';
                }
            }
            /*@fallthrough@*/
        case 'd':
            success = to_double(field, &real, sci, decimal);
            if (success) {
                return 'd';
            }
            /*@fallthrough@*/
        case 'z':
            success = to_complex(field, &real, &imag, sci, decimal);
            if (success) {
                return 'z';
            }
            if (prev_type == 'd' || prev_type == 'z') {
                /* Don't bother trying to parse a date in this case. */
                break;
            }
            /*@fallthrough@*/
        //case 'U':
        //    if ((datetime_fmt != NULL) && (strptime(field, datetime_fmt, &tm))) {
        //        return 'U';
        //    }
    }
    while (*field == ' ') {
        ++field;
    }
    if (!*field) {
        /* All spaces, so return prev_type */
        return prev_type;
    }
    return 'S';
}

/*
 *  char type_for_integer_range(int64_t imin, uint64_t umax)
 *
 *  Determine an appropriate type character for the given
 *  range of integers.  The function assumes imin <= 0.
 *  If imin == 0, the return value will be one of
 *      'B': 8 bit unsigned,
 *      'H': 16 bit unsigned,
 *      'I': 32 bit unsigned,
 *      'Q': 64 bit unsigned.
 *  If imin < 0, the possible return values
 *  are
 *      'b': 8 bit signed,
 *      'h': 16 bit signed,
 *      'i': 32 bit signed,
 *      'q': 64 bit signed
 *      'd': floating point double precision
 *
 */

char type_for_integer_range(int64_t imin, uint64_t umax)
{
    char type;

    if (imin == 0) {
        /* unsigned int type */
        if (umax <= UINT8_MAX) {
            type = 'B';
        }
        else if (umax <= UINT16_MAX) {
            type = 'H';
        }
        else if (umax <= UINT32_MAX) {
            type = 'I';
        }
        else {
            type = 'Q';
        }
    }
    else {
        /* int type */
        if (imin >= INT8_MIN && umax <= INT8_MAX) {
            type = 'b';
        }
        else if (imin >= INT16_MIN && umax <= INT16_MAX) {
            type = 'h';
        }
        else if (imin >= INT32_MIN && umax <= INT32_MAX) {
            type = 'i';
        }
        else if (umax <= INT64_MAX) {
            type = 'q';
        }
        else {
            /* 
             *  imin < 0 and the largest value exceeds INT64_MAX, so this
             * range can not be represented with an integer format.
             *  We'll have to convert these to floating point.
             */
            type = 'd';
        }
    }
    return type;
}
