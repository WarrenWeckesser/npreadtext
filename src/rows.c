
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
#include "blocks.h"

#define INITIAL_BLOCKS_TABLE_LENGTH 200
#define ROWS_PER_BLOCK 500

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


//
// If num_field_types is not 1, actual_num_fields must equal num_field_types.
//
size_t compute_row_size(int actual_num_fields,
                        int num_field_types, field_type *field_types)
{
    size_t row_size;

    // rowsize is the number of bytes in each "row" of the array
    // filled in by this function.
    if (num_field_types == 1) {
        row_size = actual_num_fields * field_types[0].itemsize;
    }
    else {
        row_size = 0;
        for (int k = 0; k < num_field_types; ++k) {
            row_size += field_types[k].itemsize;
        }
    }
    return row_size;
}


/*
 *  XXX Handle errors in any of the functions called by read_rows().
 *
 *  XXX Check handling of *nrows == 0.
 *
 *  Parameters
 *  ----------
 *  stream *s
 *  int *nrows
 *      On input, *nrows is the maximum number of rows to read.
 *      If *nrows is positive, `data_array` must point to the block of data
 *      where the data is to be stored.
 *      If *nrows is negative, all the rows in the file should be read, and
 *      the given value of `data_array` is ignored.  Data will be allocated
 *      dynamically in this function.
 *      On return, *nrows is the number of rows actually read from the file.
 *  int num_field_types
 *      Number of field types (i.e. the number of fields).  This is the
 *      length of the array pointed to by field_types.
 *  field_type *field_types
 *  parser_config *pconfig
 *  int32_t *usecols
 *      Pointer to array of column indices to use.
 *      If NULL, use all the columns (and ignore `num_usecols`).
 *  int num_usecols
 *      Length of the array `usecols`.  Ignored if `usecols` is NULL.
 *  int skiplines
 *  void *data_array
 *  int *num_cols
 *      The actual number of columns (or fields) of the data being returned.
 *  int *p_error_type
 *  int *p_error_lineno
 */

void *read_rows(stream *s, int *nrows,
                int num_field_types, field_type *field_types,
                parser_config *pconfig,
                int32_t *usecols, int num_usecols,
                int skiplines,
                void *data_array,
                int *num_cols,
                int *p_error_type, int *p_error_lineno)
{
    char *data_ptr;
    int current_num_fields;
    char **result;
    size_t row_size;
    size_t size;

    bool use_blocks;
    blocks_data *blks = NULL;

    int row_count;
    char word_buffer[WORD_BUFFER_SIZE];
    int tok_error_type;
    bool error_occurred;

    *p_error_type = 0;
    *p_error_lineno = 0;

    int actual_num_fields = -1;

    stream_skiplines(s, skiplines);

    if (stream_peek(s) == STREAM_EOF) {
        // There were fewer lines in the file than skiplines.
        // This is not treated as an error. The result should be an
        // empty array.
        if (*nrows < 0) {
            blocks_destroy(blks);
        }
        *nrows = 0;
        //stream_close(s, RESTORE_FINAL);

        // FIXME: NULL is not correct!
        return NULL;
    }

    row_count = 0;
    error_occurred = false;
    while (((*nrows < 0) || (row_count < *nrows)) &&
           (result = tokenize(s, word_buffer, WORD_BUFFER_SIZE, pconfig,
                              &current_num_fields, &tok_error_type)) != NULL) {
        int j, k;

        if (actual_num_fields == -1) {
            // We've deferred some of the initialization tasks to here,
            // because we've now read the first line, and we definitively
            // know how many fields (i.e. columns) we will be processing.
            if (num_field_types > 1) {
                actual_num_fields = num_field_types;
            }
            else if (usecols != NULL) {
                actual_num_fields = num_usecols;
            }
            else {
                // num_field_types is 1.  (XXX Check that it can't be 0 or neg.)
                // Set actual_num_fields to the number of fields found in the
                // first line of data.
                actual_num_fields = current_num_fields;
            }
            *num_cols = actual_num_fields;
            row_size = compute_row_size(actual_num_fields,
                                        num_field_types, field_types);

            if (usecols == NULL) {
                num_usecols = actual_num_fields;
            }

            use_blocks = false;
            if (*nrows < 0) {
                // Any negative value means "read the entire file".
                // In this case, it is assumed that *data_array is NULL
                // or not initialized. I.e. the value passed in is ignored,
                // and instead is initialized to the first block.
                use_blocks = true;
                blks = blocks_init(row_size, ROWS_PER_BLOCK, INITIAL_BLOCKS_TABLE_LENGTH);
                if (blks == NULL) {
                    // XXX Check for other clean up that might be necessary.
                    *p_error_type = ERROR_OUT_OF_MEMORY;
                    return NULL;
                }
            }
            else {
                // *nrows >= 0
                // FIXME: Ensure that *nrows == 0 is handled correctly.
                if (data_array == NULL) {
                    // The number of rows to read was given, but a memory buffer
                    // was not, so allocate one here.
                    size = *nrows * row_size;
                    data_array = malloc(size);
                    if (data_array == NULL) {
                        *p_error_type = ERROR_OUT_OF_MEMORY;
                        return NULL;
                    }
                }
                data_ptr = data_array;
            }
        }

        if (use_blocks) {
            data_ptr = blocks_get_row_ptr(blks, row_count);
            if (data_ptr == NULL) {
                blocks_destroy(blks);
                *p_error_type = ERROR_OUT_OF_MEMORY;
                return NULL;
            }
        }

        for (j = 0; j < num_usecols; ++j) {
            int error = ERROR_OK;
            int f = (num_field_types == 1) ? 0 : j;
            char typecode = field_types[f].typecode;

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
                data_ptr += field_types[f].itemsize;
            }
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

    if (use_blocks) {
        // Copy the blocks into a newly allocated contiguous array.
        data_array = blocks_to_contiguous(blks, row_count);
        blocks_destroy(blks);
    }

    //stream_close(s, RESTORE_FINAL);

    *nrows = row_count;

    return (void *) data_array;
}
