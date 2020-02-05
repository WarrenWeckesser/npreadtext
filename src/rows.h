
#ifndef _ROWS_H_
#define _ROWS_H_

#include <stdio.h>

#include "stream.h"
#include "field_type.h"
#include "parser_config.h"

#define READ_ERROR_OUT_OF_MEMORY   1

#define ANALYZE_FILE_ERROR    -1
#define ANALYZE_OUT_OF_MEMORY -2

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
                int *p_error_type, int *p_error_lineno);

#endif
