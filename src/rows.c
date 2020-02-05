
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#include "stream.h"
#include "constants.h"
#include "tokenize.h"
#include "sizes.h"
#include "conversions.h"
#include "field_type.h"
#include "rows.h"
#include "error_types.h"
#include "str_to.h"



//
//  For now, a convenient way to print an error to the console.
//  XXX Need a better design for handling invalid values encountered
//      in a file.
//
void _check_field_error(int error, stream *s, int col_index, char *typename, char *text)
{
    if (error != ERROR_OK) {
        fprintf(stderr, "line num %d, col num %d: bad %s value: '%s'\n",
                stream_linenumber(s), col_index + 1, typename, text);
    }
}

/*
 *  XXX Handle errors in any of the functions called by read_rows().
 *
 *  XXX Currently *nrows must be at least 1.
 *
 *  Parameters
 *  ----------
 *  ...
 *  num_field_types : int
 *      Number of field types (i.e. the number of fields).  This is the
 *      length of the array pointed to by field_types.
 *  ...
 *  usecols : int32_t *
 *      Pointer to array of column indices to use.
 *      If NULL, use all the columns (and ignore `num_usecols`).
 *  num_usecols : int
 *      Length of the array `usecols`.  Ignored if `usecols` is NULL.
 */

void *read_rows(stream *s, int *nrows,
                int num_field_types, field_type *field_types,
                parser_config *pconfig,
                int32_t *usecols, int num_usecols,
                int skiplines,
                void *data_array,
                int *p_error_type, int *p_error_lineno)
{
    char *data_ptr;
    int current_num_fields;
    char **result;
    int size;
    int row_count;
    int j;
    char word_buffer[WORD_BUFFER_SIZE];
    int tok_error_type;
    int error_occurred;

    *p_error_type = 0;
    *p_error_lineno = 0;

    //if (datetime_fmt == NULL || *datetime_fmt == '\0') {
    //    datetime_fmt = "%Y-%m-%d %H:%M:%S";
    //}

    size = 0;
    for (j = 0; j < num_field_types; ++j) {
        size += field_types[j].itemsize;
    }
    size *= *nrows;

    if (data_array == NULL) {
        data_array = malloc(size);
        if (data_array == NULL) {
            *p_error_type = ERROR_OUT_OF_MEMORY;
            return NULL;
        }
    }
    data_ptr = data_array;

    stream_skiplines(s, skiplines);

    if (stream_peek(s) == STREAM_EOF) {
        /* There were fewer lines in the file than skiplines. */
        /* This is not treated as an error. The result should be an empty array. */
        *nrows = 0;
        //stream_close(s, RESTORE_FINAL);
        return data_ptr;
    }

    if (usecols == NULL) {
        num_usecols = num_field_types;
    }

    row_count = 0;
    error_occurred = false;
    while ((row_count < *nrows) &&
           (result = tokenize(s, word_buffer, WORD_BUFFER_SIZE, pconfig,
                              &current_num_fields, &tok_error_type)) != NULL) {
        int j, k;

        for (j = 0; j < num_usecols; ++j) {
            int error = ERROR_OK;
            char typecode = field_types[j].typecode;

            /* k is the column index of the field in the file. */
            if (usecols == NULL) {
                k = j;
            }
            else {
                k = usecols[j];
                if (k < 0) {
                    // Python-like column indexing: k = -1 means the last column.
                    k += current_num_fields;
                }
                if ((k < 0) || (k >= current_num_fields)) {
                    // XXX handle this better
                    fprintf(stderr, "line num %d: bad field index: %d (row has %d fields)\n",
                            stream_linenumber(s), usecols[j], current_num_fields);
                    *p_error_type = ERROR_INVALID_COLUMN_INDEX;
                    *p_error_lineno = stream_linenumber(s);
                    error_occurred = true;
                    break;
                }
            }

            /* XXX Handle error != 0 in the following cases. */
            if (typecode == 'b') {
                int8_t x = (int8_t) str_to_int64(result[k], INT8_MIN, INT8_MAX, &error);
                _check_field_error(error, s, k, "int8", result[k]);
                *(int8_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;
            }
            else if (typecode == 'B') {
                uint8_t x = (uint8_t) str_to_uint64(result[k], UINT8_MAX, &error);
                _check_field_error(error, s, k, "uint8", result[k]);
                *(uint8_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;   
            }
            else if (typecode == 'h') {
                int16_t x = (int16_t) str_to_int64(result[k], INT16_MIN, INT16_MAX, &error);
                _check_field_error(error, s, k, "int16", result[k]);
                *(int16_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;
            }
            else if (typecode == 'H') {
                uint16_t x = (uint16_t) str_to_uint64(result[k], UINT16_MAX, &error);
                _check_field_error(error, s, k, "uint16", result[k]);
                *(uint16_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;    
            }
            else if (typecode == 'i') {
                int32_t x = (int32_t) str_to_int64(result[k], INT32_MIN, INT32_MAX, &error);
                _check_field_error(error, s, k, "int32", result[k]);
                *(int32_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;   
            }
            else if (typecode == 'I') {
                uint32_t x = (uint32_t) str_to_uint64(result[k], UINT32_MAX, &error);
                _check_field_error(error, s, k, "uint32", result[k]);
                *(uint32_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;   
            }
            else if (typecode == 'q') {
                int64_t x = (int64_t) str_to_int64(result[k], INT64_MIN, INT64_MAX, &error);
                _check_field_error(error, s, k, "int64", result[k]);
                *(int64_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize; 
            }
            else if (typecode == 'Q') {
                uint64_t x = (uint64_t) str_to_uint64(result[k], UINT64_MAX, &error);
                _check_field_error(error, s, k, "uint64", result[k]);
                *(uint64_t *) data_ptr = x;
                data_ptr += field_types[j].itemsize;    
            }
            else if (typecode == 'f' || typecode == 'd') {
                // Convert to float.
                double x;
                char decimal = pconfig->decimal;
                char sci = pconfig->sci;
                if ((*(result[k]) == '\0') || !to_double(result[k], &x, sci, decimal)) {
                    _check_field_error(error, s, k, "floating point", result[k]);
                    x = NAN;
                }
                if (typecode == 'f')
                    *(float *) data_ptr = (float) x;
                else
                    *(double *) data_ptr = x;
                data_ptr += field_types[j].itemsize;
            }
            // else if (typecode == 'c' || typecode == 'z') {
            //     // Convert to complex.
            //     double x, y;
            //     char decimal = pconfig->decimal;
            //     char sci = pconfig->sci;
            //     if ((*(result[k]) == '\0') || !to_complex(result[k], &x, &y, sci, decimal)) {
            //         _check_field_error(error, s, k, "complex", result[k]);
            //         x = NAN;
            //         y = x;
            //     }
            //     if (typecode == 'c') {
            //         *(float *) data_ptr = (float) x;
            //         data_ptr += field_types[j].itemsize / 2;
            //         *(float *) data_ptr = (float) y;
            //     }
            //     else {
            //         *(double *) data_ptr = x;
            //         data_ptr += field_types[j].itemsize / 2; 
            //         *(double *) data_ptr = y;
            //     }
            //     data_ptr += field_types[j].itemsize / 2;
            // }
            // else if (typecode == 'U') {
            //     // Datetime64, microseconds.
            //     struct tm tm = {0,0,0,0,0,0,0,0,0};
            //     time_t t;
            //
            //     if (strptime(result[k], datetime_fmt, &tm) == NULL) {
            //         _check_field_error(error, s, k, "datetime", result[k]);
            //         memset(data_ptr, 0, 8);
            //     }
            //     else {
            //         tm.tm_isdst = -1;
            //         t = mktime(&tm);
            //         if (t == -1) {
            //             _check_field_error(error, s, k, "datetime", result[k]);
            //             memset(data_ptr, 0, 8);
            //         }
            //         else {
            //             *(uint64_t *) data_ptr = (long long) (t - tz_offset) * 1000000L;
            //         }
            //     }
            //     data_ptr += 8;
            // }
            else {
                // String
                strncpy(data_ptr, result[k], field_types[j].itemsize);
                data_ptr += field_types[j].itemsize;
            }
        }

        free(result);

        if (error_occurred) {
            break;
        }

        ++row_count;
    }

    //stream_close(s, RESTORE_FINAL);

    *nrows = row_count;

    return (void *) data_array;
}
