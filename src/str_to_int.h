#ifndef STR_TO_INT_H
#define STR_TO_INT_H

#include "typedefs.h"
#include "parser_config.h"

#define DECLARE_TO_INT_PROTOTYPE(intw)                                          \
    intw##_t to_##intw(char32_t *field, parser_config *pconfig, int *error);    \

DECLARE_TO_INT_PROTOTYPE(int8)
DECLARE_TO_INT_PROTOTYPE(int16)
DECLARE_TO_INT_PROTOTYPE(int32)
DECLARE_TO_INT_PROTOTYPE(int64)

DECLARE_TO_INT_PROTOTYPE(uint8)
DECLARE_TO_INT_PROTOTYPE(uint16)
DECLARE_TO_INT_PROTOTYPE(uint32)
DECLARE_TO_INT_PROTOTYPE(uint64)

#endif
