
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

#define ROWS_PER_BLOCK 100

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
    size_t rowsize;
    size_t size;
    size_t numblocks;
    int current_block = 0;
    bool use_blocks;
    void **blocks;
    size_t blocksize;
    int max_block;
    int row_count;
    int j;
    char word_buffer[WORD_BUFFER_SIZE];
    int tok_error_type;
    bool error_occurred;

    *p_error_type = 0;
    *p_error_lineno = 0;

    rowsize = 0;
    for (j = 0; j < num_field_types; ++j) {
        rowsize += field_types[j].itemsize;
    }
    blocksize = rowsize * ROWS_PER_BLOCK;

    // max_block is the number of entries in the blocks array.
    // This value is doubled when the row count exceeds
    // max_block*ROWS_PER_BLOCK and the blocks array is reallocated.
    max_block = 200;
    use_blocks = false;
    if (*nrows < 0) {
        // Any negative value means "read the entire file".
        // In this case, it is assumed that *data_array is NULL
        // or not initialized. I.e. the value passed in is ignored,
        // and instead is initialized to the first block.
        use_blocks = true;
        blocks = calloc(max_block, sizeof(void *));
        if (blocks == NULL) {
            *p_error_type = ERROR_OUT_OF_MEMORY;
            return NULL;
        }
    }
    else {
        // *nrows >= 0
        // FIXME: Ensure that *nrows == 0 is handled correctly.
        if (data_array == NULL) {
            size = *nrows * rowsize;
            data_array = malloc(size);
            if (data_array == NULL) {
                *p_error_type = ERROR_OUT_OF_MEMORY;
                return NULL;
            }
        }
        data_ptr = data_array;
    }

    stream_skiplines(s, skiplines);

    if (stream_peek(s) == STREAM_EOF) {
        /* There were fewer lines in the file than skiplines. */
        /* This is not treated as an error. The result should be an empty array. */
        *nrows = 0;
        //stream_close(s, RESTORE_FINAL);
        if (use_blocks) {
            free(blocks);
        }
        // FIXME: This is not correct when use_blocks is true.
        return data_ptr;
    }

    if (usecols == NULL) {
        num_usecols = num_field_types;
    }

    row_count = 0;
    error_occurred = false;
    while (((*nrows < 0) || (row_count < *nrows)) &&
           (result = tokenize(s, word_buffer, WORD_BUFFER_SIZE, pconfig,
                              &current_num_fields, &tok_error_type)) != NULL) {
        int j, k;

        if (use_blocks) {
            int block_number = row_count / ROWS_PER_BLOCK;
            int block_offset = row_count % ROWS_PER_BLOCK;
            if (block_number >= max_block) {
                // Reached the end of the array of block pointers,
                // so reallocate with twice the number of entries.
                void *new_blocks = realloc(blocks, 2*max_block*sizeof(void *));
                if (new_blocks == NULL) {
                    // FIXME: handle this allocation failure.
                    return NULL;
                }
                blocks = new_blocks;
                for (j = max_block; j < 2*max_block; ++j) {
                    blocks[j] = NULL;
                }
                max_block = 2*max_block;
            }
            if (blocks[block_number] == NULL) {
                // Haven't allocated this block yet...
                blocks[block_number] = malloc(blocksize);
                if (blocks[block_number] == NULL) {
                    *p_error_type = ERROR_OUT_OF_MEMORY;
                    for (j = 0; j < current_block; ++j) {
                        free(blocks[current_block]);
                    }
                    free(blocks);
                    return NULL;
                }
                data_ptr = blocks[current_block];
            }
        }

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
        size_t actual_size = row_count * rowsize;
        int num_full_blocks = row_count / ROWS_PER_BLOCK;
        int num_rows_last = row_count % ROWS_PER_BLOCK;
        data_array = realloc(blocks[0], actual_size);
        for (int j = 1; j < num_full_blocks; ++j) {
            memcpy(data_array + j*blocksize, blocks[j], blocksize);
            free(blocks[j]);
        }
        if ((num_full_blocks > 0) && (num_rows_last > 0)) {
            memcpy(data_array + num_full_blocks*blocksize,
                   blocks[num_full_blocks],
                   num_rows_last*rowsize);
            free(blocks[num_full_blocks]);
        }
    }

    //stream_close(s, RESTORE_FINAL);

    *nrows = row_count;

    return (void *) data_array;
}
