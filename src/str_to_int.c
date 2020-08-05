

#include "typedefs.h"
#include "str_to.h"
#include "error_types.h"
#include "conversions.h"
#include "parser_config.h"


#define DECLARE_TO_INT(intw, INT_MIN, INT_MAX)                                  \
    intw##_t to_##intw(char32_t *field, parser_config *pconfig, int *error)     \
    {                                                                               \
        intw##_t x;                                                                 \
        int ierror = 0;                                                             \
                                                                                    \
        *error = ERROR_OK;                                                          \
                                                                                    \
        x = (intw##_t) str_to_int64(field, INT_MIN, INT_MAX, &ierror);              \
        if (ierror) {                                                               \
            if (pconfig->allow_float_for_int) {                                     \
                double fx;                                                          \
                char32_t decimal = pconfig->decimal;                                \
                char32_t sci = pconfig->sci;                                        \
                if ((*field == '\0') || !to_double(field, &fx, sci, decimal)) {     \
                    *error = ERROR_BAD_FIELD;                                       \
                }                                                                   \
                else {                                                              \
                    x = (intw##_t) fx;                                              \
                }                                                                   \
            }                                                                       \
            else {                                                                  \
                *error = ERROR_BAD_FIELD;                                           \
            }                                                                       \
        }                                                                           \
        return x;                                                                   \
    }                                                                               \

#define DECLARE_TO_UINT(uintw, UINT_MAX)                                            \
    uintw##_t to_##uintw(char32_t *field, parser_config *pconfig, int *error)       \
    {                                                                               \
        uintw##_t x;                                                                \
        int ierror = 0;                                                             \
                                                                                    \
        *error = ERROR_OK;                                                          \
                                                                                    \
        x = (uintw##_t) str_to_uint64(field, UINT_MAX, &ierror);                    \
        if (ierror) {                                                               \
            if (pconfig->allow_float_for_int) {                                     \
                double fx;                                                          \
                char32_t decimal = pconfig->decimal;                                \
                char32_t sci = pconfig->sci;                                        \
                if ((*field == '\0') || !to_double(field, &fx, sci, decimal)) {     \
                    *error = ERROR_BAD_FIELD;                                       \
                }                                                                   \
                else {                                                              \
                    x = (uintw##_t) fx;                                             \
                }                                                                   \
            }                                                                       \
            else {                                                                  \
                *error = ERROR_BAD_FIELD;                                           \
            }                                                                       \
        }                                                                           \
        return x;                                                                   \
    }                                                                               \

DECLARE_TO_INT(int8, INT8_MIN, INT8_MAX)
DECLARE_TO_INT(int16, INT16_MIN, INT16_MAX)
DECLARE_TO_INT(int32, INT32_MIN, INT32_MAX)
DECLARE_TO_INT(int64, INT64_MIN, INT64_MAX)

DECLARE_TO_UINT(uint8, UINT8_MAX)
DECLARE_TO_UINT(uint16, UINT16_MAX)
DECLARE_TO_UINT(uint32, UINT32_MAX)
DECLARE_TO_UINT(uint64, UINT64_MAX)
