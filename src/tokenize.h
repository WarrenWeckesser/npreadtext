
#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "stream.h"
#include "parser_config.h"

char **tokenize(stream *fb, char *word_buffer, int word_buffer_size,
                parser_config *pconfg,
                int *p_num_fields,
                int *p_error_type);

#endif
