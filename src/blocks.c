
// Pure C, no Python API used.
// With MAINTEST defined, this file has a main() that runs some
// code checks.

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
            // this case won't occur, but if the code ever starts
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
// A special resize function, only for data with the same dtype for each
// field (typically 'S' or 'U').  This function resizes the fields from the
// current itemsize (b->row_size / num_fields) to the given new_itemsize.
// The function assumes that new_itemsize is larger than the current itemsize.
//
int
blocks_uniform_resize(blocks_data *b, size_t num_fields, size_t new_itemsize)
{
    int current_row_size = b->row_size;
    int current_itemsize = current_row_size / num_fields;
    int new_row_size = num_fields * new_itemsize;
    char **blocks = b->block_table;
    size_t new_block_size = new_row_size * b->rows_per_block;

    for (int i = 0; i < b->block_table_length; ++i) {
        char *current_block = blocks[i];
        if (current_block == NULL) {
            continue;
        }
        char *new_block = calloc(new_block_size, 1);
        if (new_block == NULL) {
            // XXX Deal with this... maybe free everything
            // and return an error code?
            return -1;
        }
        for (size_t row = 0; row < b->rows_per_block; ++row) {
            for (size_t col = 0; col < num_fields; ++col) {
                memcpy(new_block + row*new_row_size + col*new_itemsize,
                       current_block + row*current_row_size + col*current_itemsize,
                       current_itemsize);
            }
        }
        free(current_block);
        blocks[i] = new_block;
    }
    b->row_size = new_row_size;
    return 0;
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


#ifdef TESTMAIN

#include <stdio.h>

int main(int argc, char *argv[])
{
    int row_size = 12;
    int rows_per_block = 4;
    int block_table_length = 3;

    int num_rows = 26;

    blocks_data *b = blocks_init(row_size, rows_per_block, block_table_length);
    if (b == NULL) {
        fprintf(stderr, "blocks_init returned NULL\n");
        exit(-1);
    }

    for (size_t k = 0; k < num_rows; ++k) {
        char *ptr = blocks_get_row_ptr(b, k);
        if (ptr == NULL) {
            fprintf(stderr, "blocks_get_row_ptr(b, %zu) returned NULL\n", k);
            exit(-1);
        }
        memset(ptr, 'A' + k, row_size - 1);
        ptr[row_size-1] = '\0';
    }

    printf("b->block_table_length = %d\n", b->block_table_length);
    printf("b->block_table:\n");
    for (size_t k = 0; k < b->block_table_length; ++k) {
        char *p = b->block_table[k];
        printf("%3zu  %llu\n", k, (long long unsigned) p);
    }

    char *data = blocks_to_contiguous(b, num_rows);
    if (data == NULL) {
        fprintf(stderr, "blocks_to_contiguous returned NULL\n");
        exit(-1);
    }

    printf("contiguous data:\n");
    for (size_t k = 0; k < num_rows; ++k) {
        char *p = data + row_size*k;
        printf("%3zu '%s'\n", k, p);
    }

    free(data);
    blocks_destroy(b);

    return 0;
}

#endif
