
#ifndef _FIELD_TYPE_H_
#define _FIELD_TYPE_H_

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
    //     s : character
    //     U : 64 bit datetime (not implemented)
    char typecode;

    // itemsize:
    //   Size of field, in bytes.  In theory this would only be
    //   needed for the 's' type code, but it is expected to be
    //   correctly filled in for all the types.
    int itemsize;

} field_type;

#endif
