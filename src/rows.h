
#ifndef _ROWS_H_
#define _ROWS_H_

#include <stdio.h>

#include "stream.h"
#include "field_type.h"
#include "parser_config.h"

//
// This structure holds information about errors arising
// in read_rows().
//
typedef struct _read_error {
    int error_type;
    int line_number;
    int field_number;
    int char_position;
    char typecode;
    // int itemsize;  // not sure this is needed.
    int32_t column_index; // for ERROR_INVALID_COLUMN_INDEX;
} read_error_type;

/*
int analyze(FILE *f, parser_config *pconfig, int skiplines, int numrows,
               char *datetime_fmt, int *num_fields, field_type **field_types);

int count_fields(FILE *f, parser_config *pconfig, int skiprows);
int count_rows(FILE *f, parser_config *pconfig);
*/

void *read_rows(stream *s, int *nrows,
                int num_field_types, field_type *field_types,
                parser_config *pconfig,
                //char *datetime_fmt,
                //int tz_offset,
                int *usecols, int num_usecols,
                int skiplines,
                void *data_array,
                int *num_cols,
                read_error_type *read_error);

#endif
