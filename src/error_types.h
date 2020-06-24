#ifndef ERROR_TYPES_H
#define ERROR_TYPES_H

/*
 *  Common set of error types for the read_rows() and tokenize()
 *  functions.
 */

#define ERROR_OUT_OF_MEMORY             1
#define ERROR_INVALID_COLUMN_INDEX     10
#define ERROR_CHANGED_NUMBER_OF_FIELDS 12
#define ERROR_TOO_MANY_CHARS           21
#define ERROR_TOO_MANY_FIELDS          22
#define ERROR_NO_DATA                  23
#define ERROR_BAD_FIELD                30
#define ERROR_CONVERTER_FAILED         40

#endif
