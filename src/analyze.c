
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "tokenize.h"
#include "sizes.h"
#include "field_types.h"
#include "analyze.h"
#include "type_inference.h"
#include "stream.h"


typedef struct {

    // For integer types, the lower bound of the values.
    int64_t imin;

    // For integer types, the upper bound of the values.
    uint64_t umax;

} integer_range;


int enlarge_ranges(int new_num_fields, int num_fields, integer_range **ranges)
{
    int nbytes;
    integer_range *new_ranges;

    nbytes = new_num_fields * sizeof(integer_range);
    new_ranges = (integer_range *) realloc(*ranges, nbytes);
    if (new_ranges == NULL) {
        free(*ranges);
        *ranges = NULL;
        return -1;
    }
    for (int k = num_fields; k < new_num_fields; ++k) {
        new_ranges[k].imin = 0;
        new_ranges[k].umax = 0;
    }
    *ranges = new_ranges;
    return 0;
}

int enlarge_type_tracking_arrays(int new_num_fields, int num_fields,
                                 field_type **types, integer_range **ranges)
{
    int status1 = field_types_grow(new_num_fields, num_fields, types);
    int status2 = enlarge_ranges(new_num_fields, num_fields, ranges);
    if (status1 != 0 || status2 != 0) {
        return -1;
    }
    return 0;
}

/*
 *  Parameters
 *  ----------
 *  ...
 *  skiplines : int
 *      Number of text lines to skip before beginning to analyze the rows.
 *  numrows : int
 *      maximum number of rows to analyze (currently not implemented)
 *  datetime_fmt : char *
 *      If NULL, analyze does not try to parse a field as a date.
 *      Otherwise, datetime_fmt must point to a format string that will
 *      be used by strptime to parse fields.  If strptime succeeds, the
 *      field is classified as type 'U' (datetime).
 *      XXX This design is probably too restrictive, esp. considering that
 *          a very common case is to have two fields, one a date and one
 *          a time, e.g.:
 *              1/2/2010,12:34:56,etc
 *
 *  Return value
 *  ------------
 *  row_count > 0: number of rows. row_count might be less than numrows if the
 *      end of the file is reached.
 *  ANALYZE_FILE_ERROR:    unable to create a file buffer.
 *  ANALYZE_OUT_OF_MEMORY: out of memory (malloc failed)
 *  ...
 */

int analyze(stream *s, parser_config *pconfig, int skiplines, int numrows,
            int *p_num_fields, field_type **p_field_types)
{
    int row_count = 0;
    int num_fields = 0;
    field_type *types = NULL;
    integer_range *ranges = NULL;

    char decimal = pconfig->decimal;
    char sci = pconfig->sci;

    stream_skiplines(s, skiplines);
    if (stream_peek(s) == STREAM_EOF) {
        // Reached the end of the file while skipping lines.
        // stream_close(s, RESTORE_INITIAL);
        return 0;
    }

    char *word_buffer = malloc(WORD_BUFFER_SIZE);
    if (word_buffer == NULL) {
        return ANALYZE_OUT_OF_MEMORY;
    }

    // In this loop, types[k].itemsize will track the largest field length
    // encountered for field k, regardless of the apparent type of the field
    // (since we don't know the field type until we've seen the whole
    // file).
    // ranges[k].imin tracks the most negative integer seen in the field.
    // (If all the values are positive, ranges[k].imin will be 0.)
    // ranges[k].umax tracks the largest positive integer seen in the field.
    // (If all the values are negative, ranges[k].umax will be 0.)
    while (row_count != numrows) {
        int new_num_fields;
        int tok_error_type;
        char **result;

        result = tokenize(s, word_buffer, WORD_BUFFER_SIZE,
                          pconfig, &new_num_fields, &tok_error_type);
        if (result == NULL) {
            break;
        }

        if (new_num_fields > num_fields) {
            // The first row, or a row with more fields than previously seen...
            int status = enlarge_type_tracking_arrays(new_num_fields, num_fields,
                                                      &types, &ranges);
            if (status != 0) {
                // If this occurs, types and ranges have been freed in
                // enlarge_type_tracking_array().
                free(result);
                free(word_buffer);
                //fb_del(fb, RESTORE_INITIAL);
                return ANALYZE_OUT_OF_MEMORY;
            }
            num_fields = new_num_fields;
        }

        for (int k = 0; k < new_num_fields; ++k) {
            char typecode;
            int field_len;
            int64_t imin;
            uint64_t umax;
            typecode = classify_type(result[k], decimal, sci, &imin, &umax,
                                     types[k].typecode);
            if (typecode == 'q' && imin < ranges[k].imin) {
                ranges[k].imin = imin;
            }
            if (typecode == 'Q' && umax > ranges[k].umax) {
                ranges[k].umax = umax;
            }
            if (typecode != '*') {
                types[k].typecode = typecode;
            }
            field_len = strlen(result[k]);
            if (field_len > types[k].itemsize) {
                types[k].itemsize = field_len;
            }
        }
        free(result);
        ++row_count;
    }

    free(word_buffer);

    // At this point, any field that contained only unsigned integers
    // or only integers (some negative) has been classified as typecode='Q'
    // or typecode='q', respectively.  Now use the integer ranges that were
    // found to refine these classifications.
    // We also "fix" the itemsize field for any non-string fields (even though
    // the itemsize of non-strings is implicit in the typecode).

    for (int k = 0; k < num_fields; ++k) {
        char typecode = types[k].typecode;
        if (typecode == 'q' || typecode == 'Q') {
            // Integer type.  Use imin and umax to refine the type.
            types[k].typecode = type_for_integer_range(ranges[k].imin, ranges[k].umax);
        }
        // Note: 'f' and 'c' are included among the options below, but currently,
        // these will not be assigned in the code above.  Eventually, the user might
        // be able to specify the default sizes for numerical field, and the code
        // above *might* change for this.  For now, there is no harm in including these
        // typecodes in the switch statement.
        switch (types[k].typecode) {
            case 'b':
            case 'B':
                types[k].itemsize = 1;
                break;
            case 'h':
            case 'H':
                types[k].itemsize = 2;
                break;
            case 'i':
            case 'I':
            case 'f':
                types[k].itemsize = 4;
                break;
            case 'q':
            case 'Q':
            case 'd':
            case 'c':
            case 'U':
                types[k].itemsize = 8;
                break;
            case 'z':
                types[k].itemsize = 16;
                break;
        }
    }

    free(ranges);

    *p_num_fields = num_fields;
    *p_field_types = types;

    //fb_del(fb, RESTORE_INITIAL);

    return row_count;
}
