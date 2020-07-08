
#ifndef _PARSER_CONFIG_H_
#define _PARSER_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>
#include "typedefs.h"

typedef struct _parser_config {

    /*
     *  Field delimiter character.
     *  Typically ',', ' ', '\t', or '\0'.
     *  '\0' means each contiguous span of spaces is one delimiter.
     */
    char32_t delimiter;

    /*
     *  Character used to quote fields.
     *  Typically '"' or '\''.
     *  To disable the special handling of a quote character, set this to a
     *  value that doesn't occur in the input file (e.g. '\0').
     */
    char32_t quote;

    /*
     *  Ignore spaces at the beginning of a field.  Only relevant for
     *  text fields, and only when the delimiter is not whitespace.
     *  Spaces in quotes are never ignored.
     */
    bool ignore_leading_spaces;

    /*
     *  Ignore spaces at the end of a field.  Only relevant for
     *  text fields, and only when the delimiter is not whitespace.
     *  Spaces in quotes are never ignored.
     */
    bool ignore_trailing_spaces;

    /*
     *  Ignore lines that are all spaces.  Only used when the delimiter
     *  is \0.
     */
    bool ignore_blank_lines;

    /*
     *  Character(s) that indicates the start of a comment.
     *  Typically '#', '%' or ';'.
     *  When encountered in a line and not inside quotes, all character
     *  from the comment character(s) to the end of the line are ignored.
     */
    char32_t comment[2];

    /*
     *  A boolean value (0 or 1).  If 1, quoted fields may span
     *  more than one line.  For example, the following
     *      100, 200, "FOO
     *      BAR"
     *  is one "row", containing three fields: 100, 200 and "FOO\nBAR".
     *  If 0, the parser considers an unclosed quote to be an error. (XXX Check!)
     */
    bool allow_embedded_newline;

    /*
     *  The decimal point character.
     *  Most commonly '.', but ',' is sometimes used.
     */
    char32_t decimal;

    /*
     *  The character used to indicate the exponent in scientific notation.
     *  Typically 'E' or 'e', but 'D' (or 'd') are sometimes used (mainly in
     *  Fortran code).  When parsing, the case is ignored.
     */
    char32_t sci;

    /*
     *  If strict_num_fields is True, all rows in the file are expected
     *  to have the same number of fields.  When analyzing a file, if any row
     *  is found to have a different number of fields than the first row, an error
     *  is returned.  When the data types of all the fields are provided by the
     *  user, an error is returned if any row doesn't have the same number of fields.
     *
     *  If strict_num_fields is False, then if the field is parsed automatically,
     *  the number of fields of the result will be the maximum of the number of
     *  fields in the rows.  Rows with fewer than the maximum will be extended
     *  with missing values.
     */
     bool strict_num_fields;

} parser_config;

parser_config default_parser_config(void);

#endif
