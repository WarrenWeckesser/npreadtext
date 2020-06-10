
#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include "typedefs.h"
#include "stream.h"
#include "parser_config.h"

char32_t **tokenize(stream *fb, char32_t *word_buffer, int word_buffer_size,
                    parser_config *pconfg,
                    int *p_num_fields,
                    int *p_error_type);

#endif
