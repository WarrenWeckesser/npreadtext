
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "field_types.h"


field_type *field_types_create(int num_field_types, char *codes, int32_t *sizes)
{
    field_type *ft;

    ft = malloc(num_field_types * sizeof(field_type));
    if (ft == NULL) {
        return NULL;
    }
    for (int i = 0; i < num_field_types; ++i) {
        ft[i].typecode = codes[i];
        ft[i].itemsize = sizes[i];
    }
    return ft;
}

void field_types_fprintf(FILE *out, int num_field_types, field_type *ft)
{
    for (int i = 0; i < num_field_types; ++i) {
        fprintf(out, "ft[%d].typecode = %c, .itemsize = %d\n",
                     i, ft[i].typecode, ft[i].itemsize);
    }
}


bool field_types_is_homogeneous(int num_field_types, field_type *ft)
{
    bool homogeneous = true;
    for (int k = 1; k < num_field_types; ++k) {
        if ((ft[k].typecode != ft[0].typecode) ||
                (ft[k].itemsize != ft[0].itemsize)) {
            homogeneous = false;
            break;
        }
    }
    return homogeneous;
}


int32_t field_types_total_size(int num_field_types, field_type *ft)
{
    int32_t size = 0;
    for (int k = 0; k < num_field_types; ++k) {
        size += ft[k].itemsize;
    }
    return size;
}


//
// *ft must be a pointer to field_type that is either NULL or
// previously malloc'ed.
// The function assumes that new_num_field_types > num_field_types.
//
int field_types_grow(int new_num_field_types, int num_field_types, field_type **ft)
{
    size_t nbytes;
    field_type *new_ft;

    nbytes = new_num_field_types * sizeof(field_type);
    new_ft = (field_type *) realloc(*ft, nbytes);
    if (new_ft == NULL) {
        free(*ft);
        *ft = NULL;
        return -1;
    }
    for (int k = num_field_types; k < new_num_field_types; ++k) {
        new_ft[k].typecode = '*';
        new_ft[k].itemsize = 0;
    }
    *ft = new_ft;

    return 0;
}


char *typecode_to_str(char typecode)
{
    char *typ;

    switch (typecode) {
        case 'b': typ = "int8"; break;
        case 'B': typ = "uint8"; break;
        case 'h': typ = "int16"; break;
        case 'H': typ = "uint16"; break;
        case 'i': typ = "int32"; break;
        case 'I': typ = "uint32"; break;
        case 'q': typ = "int64"; break;
        case 'Q': typ = "uint64"; break;
        case 'f': typ = "float32"; break;
        case 'd': typ = "float64"; break;
        case 'c': typ = "complex64"; break;
        case 'z': typ = "complex128"; break;
        case 'S': typ = "S"; break;
        case 'U': typ = "U"; break;
        default:  typ = "unknown";
    }

    return typ;
}
