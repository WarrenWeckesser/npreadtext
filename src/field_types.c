
//
// Pure C -- no Python API.
// With TESTMAIN defined, this file has a main() that runs some "tests"
// (actually it just prints some stuff, to be verified by the user).
//

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "field_types.h"


field_type *field_types_create(int num_field_types,
                               const char *codes,
                               const int32_t *sizes)
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

void field_types_fprintf(FILE *out, int num_field_types, const field_type *ft)
{
    for (int i = 0; i < num_field_types; ++i) {
        fprintf(out, "ft[%d].typecode = %c, .itemsize = %d\n",
                     i, ft[i].typecode, ft[i].itemsize);
    }
}


bool field_types_is_homogeneous(int num_field_types, const field_type *ft)
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


int32_t field_types_total_size(int num_field_types, const field_type *ft)
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


//
// Build a comma separated string representation of the dtype.
// If cols == NULL, it is not used.  Otherwise it is an array
// of length num_cols that gives the index into ft to use.
//
char *field_types_build_str(int num_cols, const int32_t *cols, bool homogeneous,
                            const field_type *ft)
{
    char *dtypestr;
    size_t len;

    // Precompute the length of the string.
    // len = ...
    len = num_cols * 8;   // FIXME: crude estimate
    dtypestr = malloc(len);

    if (dtypestr == NULL) {
        return NULL;
    }

    // Fill in the string
    int p = 0;
    for (int j = 0; j < num_cols; ++j) {
        int k;
        if (cols == NULL) {
            // No indirection via cols.
            k = j;
        }
        else {
            // FIXME Values in usecols have not been validated!!!
            k = cols[j];
        }
        if (j > 0) {
            dtypestr[p++] = ',';
        }
        dtypestr[p++] = ft[j].typecode;
        if (ft[j].typecode == 'S') {
            int nc = snprintf(dtypestr + p, len - p - 1, "%d", ft[j].itemsize);
            p += nc;
        }
        else if (ft[j].typecode == 'U') {
            int nc = snprintf(dtypestr + p, len - p - 1, "%d", ft[j].itemsize / 4);
            p += nc;
        }
        if (homogeneous) {
            break;
        }
    }
    dtypestr[p] = '\0';
    return dtypestr;
}


#ifdef TESTMAIN

void show_field_types(int num_fields, field_type *ft)
{
    field_types_fprintf(stdout, num_fields, ft);

    bool homogeneous = field_types_is_homogeneous(num_fields, ft);
    printf("homogeneous = %s\n", homogeneous ? "true" : "false");

    int32_t total_size = field_types_total_size(num_fields, ft);
    printf("total_size = %d\n", total_size);

    char *dtypestr = field_types_build_str(num_fields, NULL, homogeneous, ft);
    printf("dtypestr = '%s'\n", dtypestr);

    free(dtypestr);
}


int main(int argc, char *argv[])
{
    char *codes = "ffHHSU";
    int32_t sizes[] = {8, 8, 2, 2, 4, 48};
    int num_fields = sizeof(sizes) / sizeof(sizes[0]);

    field_type *ft = field_types_create(num_fields, codes, sizes);
    printf("ft:\n");
    show_field_types(num_fields, ft);

    int status = field_types_grow(num_fields + 2, num_fields, &ft);
    ft[num_fields].typecode = 'b';
    ft[num_fields].itemsize = 1;
    ++num_fields;
    ft[num_fields].typecode = 'f';
    ft[num_fields].itemsize = 4;
    ++num_fields;

    printf("\n");
    printf("ft, after grow:\n");
    show_field_types(num_fields, ft);

    free(ft);

    return 0;
}

#endif
