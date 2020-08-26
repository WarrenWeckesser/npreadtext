
#ifndef _FIELD_TYPES_H_
#define _FIELD_TYPES_H_

#include <stdint.h>
#include <stdbool.h>


typedef struct _field_type {
    // typecode:
    //   Format characters for data type of a field (mostly compatible with
    //   the codes in the python struct module):
    //     b : 8 bit signed char
    //     B : 8 bit unsigned char
    //     h : 16 bit signed integer
    //     H : 16 bit unsigned integer
    //     i : 32 bit signed integer
    //     I : 32 bit unsigned integer
    //     q : 64 bit signed integer
    //     Q : 64 bit unsigned integer
    //     f : 32 bit floating point
    //     d : 64 bit floating point
    //     c : 32 bit complex (real and imag are each 32 bit) (not implemented)
    //     z : 64 bit complex (real and imag are each 64 bit) (not implemented)
    //     S : character string (1 character == 1 byte)
    //     U : Unicode string (32 bit codepoints)
    char typecode;

    // itemsize:
    //   Size of field, in bytes.  In theory this would only be
    //   needed for the 'S' or 'U' type codes, but it is expected to be
    //   correctly filled in for all the types.
    int32_t itemsize;

} field_type;


field_type *field_types_create(int num_field_types, const char *codes, const int *sizes);
void field_types_fprintf(FILE *out, int num_field_types, const field_type *ft);
bool field_types_is_homogeneous(int num_field_types, const field_type *ft);
int32_t field_types_total_size(int num_field_types, const field_type *ft);
int field_types_grow(int new_num_field_types, int num_field_types, field_type **ft);
char *field_types_build_str(int num_cols, const int32_t *cols, bool homogeneous,
                            const field_type *ft);

char *typecode_to_str(char typecode);

#endif
