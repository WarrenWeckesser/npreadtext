#include <stdlib.h>
#include <string.h>
#include "blocks.h"


//
// int row_size
//     The number of bytes in each row.
// int rows_per_block
//     The number of rows in each block
// int block_table_length
//     The initial length of the table of pointers to blocks.
//
// Returns NULL if a memory allocation fails.
//

blocks_data *
blocks_init(int row_size, int rows_per_block, int block_table_length)
{
    blocks_data *b;

    b = malloc(sizeof(blocks_data));
    if (b == NULL) {
        return b;
    }
    b->row_size = row_size;
    b->block_table_length = block_table_length;
    b->rows_per_block = rows_per_block;
    b->block_table = calloc(block_table_length, sizeof(void *));
    if (b->block_table == NULL) {
        free(b);
        return NULL;
    }
    return b;
}

//
// b must have been created by blocks_init(...).
//
void
blocks_destroy(blocks_data *b)
{
    for (int k = 0; k < b->block_table_length; ++k) {
        free(b->block_table[k]);
    }
    free(b->block_table);
    free(b);
}

//
// Returns NULL if a memory allocation fails.
//

char *
blocks_get_row_ptr(blocks_data *b, size_t k)
{
    int rows_per_block = b->rows_per_block;
    int block_table_length = b->block_table_length;

    int block_number = k / rows_per_block;
    int block_offset = k % rows_per_block;

    if (block_number >= block_table_length) {
        // The requested block is beyond the end of the array
        // of block pointers, so reallocate a bigger table.
        int new_block_table_length = 2 * block_table_length;
        if (block_number >= new_block_table_length) {
            // When accessing the rows sequentially from the beginning,
            // this case won't occur, but if the code evers starts
            // randomly accessing rows, we make sure a valid table length
            // is used.  (Minimal length would be block_number; add 1 more
            // for no particular reason.)
            new_block_table_length = block_number + 1;
        }
        char **new_block_table = realloc(b->block_table,
                                         new_block_table_length * sizeof(char *));
        if (new_block_table == NULL) {
            return NULL;
        }
        // Ensure that all the new pointers in the extended table are NULL.
        // We'll call free with these when the data structure is destroyed.
        for (int j = block_table_length; j < new_block_table_length; ++j) {
            new_block_table[j] = NULL;
        }
        b->block_table = new_block_table;
        b->block_table_length = new_block_table_length;
    }

    if (b->block_table[block_number] == NULL) {
        // Haven't allocated this block yet...
        int block_size = b->row_size * rows_per_block;
        b->block_table[block_number] = malloc(block_size);
        if (b->block_table[block_number] == NULL) {
            return NULL;
        }
    }

    return b->block_table[block_number] + block_offset * b->row_size;
}


//
// Copy the first num_rows from the blocks data structure
// to a newly allocated contiguous block of memory.
//
// Returns NULL if a memory allocation fails.
//

char *
blocks_to_contiguous(blocks_data *b, size_t num_rows)
{
    size_t actual_size = num_rows * b->row_size;
    char *data = malloc(actual_size);
    if (data == NULL) {
        return NULL;
    }
    int num_full_blocks = num_rows / b->rows_per_block;
    int num_rows_last = num_rows % b->rows_per_block;
    size_t block_size = b->row_size * b->rows_per_block;
    for (int j = 0; j < num_full_blocks; ++j) {
        memcpy(data + j * block_size, b->block_table[j], block_size);
    }
    if (num_rows_last > 0) {
        memcpy(data + num_full_blocks * block_size,
               b->block_table[num_full_blocks],
               num_rows_last * b->row_size);
    }
    return data;
}
