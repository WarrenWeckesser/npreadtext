
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
 *  read_error_type *read_error
 *      Information about errors detected in read_rows()
 */

void *read_rows(stream *s, int *nrows,
                int num_field_types, field_type *field_types,
                parser_config *pconfig,
                int32_t *usecols, int num_usecols,
                int skiplines,
                void *data_array,
                int *num_cols,
                read_error_type *read_error)
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

    int actual_num_fields = -1;

    stream_skiplines(s, skiplines);

    if (stream_peek(s) == STREAM_EOF) {
        // There were fewer lines in the file than skiplines.
        // This is not treated as an error. The result should be an
        // empty array.

        //stream_close(s, RESTORE_FINAL);

        if (*nrows < 0) {
            *nrows = 0;
            return NULL;
        }
        else {
            *nrows = 0;
            return data_array;
        }
    }

    row_count = 0;
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
                    read_error->error_type = ERROR_OUT_OF_MEMORY;
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
                        read_error->error_type = ERROR_OUT_OF_MEMORY;
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
                read_error->error_type = ERROR_OUT_OF_MEMORY;
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
                    read_error->error_type = ERROR_INVALID_COLUMN_INDEX;
                    read_error->line_number = stream_linenumber(s) - 1;
                    read_error->column_index = usecols[j];
                    break;
                }
            }

            read_error->error_type = 0;
            read_error->line_number = stream_linenumber(s) - 1;
            read_error->field_number = k;
            read_error->char_position = -1; // FIXME
            read_error->typecode = typecode;

            /* XXX Handle error != 0 in the following cases. */
            if (typecode == 'b') {
                int8_t x = 0;
                if (k < current_num_fields) {
                    x = (int8_t) str_to_int64(result[k], INT8_MIN, INT8_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(int8_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'B') {
                uint8_t x = 0;
                if (k < current_num_fields) {
                    x = (uint8_t) str_to_uint64(result[k], UINT8_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(uint8_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'h') {
                int16_t x = 0;
                if (k < current_num_fields) {
                    x = (int16_t) str_to_int64(result[k], INT16_MIN, INT16_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(int16_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'H') {
                uint16_t x = 0;
                if (k < current_num_fields) {
                    x = (uint16_t) str_to_uint64(result[k], UINT16_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(uint16_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'i') {
                int32_t x = 0;
                if (k < current_num_fields) {
                    x = (int32_t) str_to_int64(result[k], INT32_MIN, INT32_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(int32_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'I') {
                uint32_t x = 0;
                if (k < current_num_fields) {
                    x = (uint32_t) str_to_uint64(result[k], UINT32_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(uint32_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'q') {
                int64_t x = 0;
                if (k < current_num_fields) {
                    x = (int64_t) str_to_int64(result[k], INT64_MIN, INT64_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(int64_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'Q') {
                uint64_t x = 0;
                if (k < current_num_fields) {
                    x = (uint64_t) str_to_uint64(result[k], UINT64_MAX, &error);
                    if (error) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                *(uint64_t *) data_ptr = x;
                data_ptr += field_types[f].itemsize;
            }
            else if (typecode == 'f' || typecode == 'd') {
                // Convert to float.
                double x = NAN;
                if (k < current_num_fields) {
                    char decimal = pconfig->decimal;
                    char sci = pconfig->sci;
                    if ((*(result[k]) == '\0') || !to_double(result[k], &x, sci, decimal)) {
                        read_error->error_type = ERROR_BAD_FIELD;
                        break;
                    }
                }
                if (typecode == 'f') {
                    *(float *) data_ptr = (float) x;
                }
                else {
                    *(double *) data_ptr = x;
                }
                data_ptr += field_types[f].itemsize;
            }
            else {
                // String
                memset(data_ptr, 0, field_types[f].itemsize);
                if (k < current_num_fields) {
                    strncpy(data_ptr, result[k], field_types[f].itemsize);
                }
                data_ptr += field_types[f].itemsize;
            }
        }

        free(result);

        if (read_error->error_type != 0) {
            break;
        }

        ++row_count;
    }

    if (use_blocks) {
        if (read_error->error_type == 0) {
            // No error.
            // Copy the blocks into a newly allocated contiguous array.
            data_array = blocks_to_contiguous(blks, row_count);
        }
        blocks_destroy(blks);
    }

    //stream_close(s, RESTORE_FINAL);

    *nrows = row_count;

    if (read_error->error_type) {
        return NULL;
    }
    return (void *) data_array;
}
