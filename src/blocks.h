#ifndef _BLOCKS_H_
#define _BLOCKS_H_


typedef struct _blocks_data
{
    int row_size;
    int rows_per_block;
    int block_table_length;
    char **block_table;
} blocks_data;


blocks_data *
blocks_init(int row_size,  int rows_per_block, int block_table_length);

void
blocks_destroy(blocks_data *b);

char *
blocks_get_row_ptr(blocks_data *b, size_t k);

int
blocks_uniform_resize(blocks_data *b, size_t num_fields, size_t new_itemsize);

char *
blocks_to_contiguous(blocks_data *b, size_t num_rows);

#endif
