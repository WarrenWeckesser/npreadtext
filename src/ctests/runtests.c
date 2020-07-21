


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <complex.h>
#include "../typedefs.h"
#include "../blocks.h"
#include "../field_types.h"
#include "../conversions.h"
#include "../str_to.h"
#include "../type_inference.h"

#include "ctestify.h"

#define ALLOW_PARENS true

#define TEST_TO_LONGLONG(value)                                 \
    str_to_char32(s, #value);                                   \
    status = to_longlong(s, &m);                                \
    assert_equal_int(results, status, 1,                        \
                     "bad return status from to_longlong()");   \
    assert_equal_longlong(results, m, value##LL,                \
                          "incorrect conversion to long long");


void str_to_char32(char32_t *w, char *s)
{
    size_t k = 0;
    while (s[k]) {
        w[k] = s[k];
        ++k;
    }
    w[k] = 0;
}


void test_conversions(test_results *results)
{
    char32_t s[64];
    double x;
    int status;

    str_to_char32(s, "1.25");
    status = to_double(s, &x, 'e', '.');
    assert_equal_int(results, status, 1, "bad return status from to_double()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to double");

    str_to_char32(s, "1,25");
    status = to_double(s, &x, 'e', ',');
    assert_equal_int(results, status, 1, "bad return status from to_double()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to double");

    str_to_char32(s, "1.25e0");
    status = to_double(s, &x, 'E', '.');
    assert_equal_int(results, status, 1, "bad return status from to_double()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to double");

    str_to_char32(s, "1.25D0");
    status = to_double(s, &x, 'D', '.');
    assert_equal_int(results, status, 1, "bad return status from to_double()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to double");

    long long m;
    TEST_TO_LONGLONG(987654321)
    TEST_TO_LONGLONG(-1)
    TEST_TO_LONGLONG(9223372036854775807)
    TEST_TO_LONGLONG(-9223372036854775807)
}

void test_complex_conversion(test_results *results)
{
    char32_t s[64];
    double x, y;
    int status;

    str_to_char32(s, "1.25+2.0j");
    status = to_complex(s, &x, &y, 'E', '.', 'j', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, 2.0, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "1.25+-2000i");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, -2000.0, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "1.25");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, 0.0, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "1.25i");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 0.0, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, 1.25, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "1.25+-2.0e1i");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 1.25, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, -20.0, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "1.0-2.0i");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 1.0, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, -2.0, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "8.125e03-.5e1i");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 8125.0, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, -5.0, "incorrect conversion to complex (imag part)");

    str_to_char32(s, "(8.125e03-.5e1i)");
    status = to_complex(s, &x, &y, 'E', '.', 'i', ALLOW_PARENS);
    assert_equal_int(results, status, 1, "bad return status from to_complex()");
    assert_equal_double(results, x, 8125.0, "incorrect conversion to complex (real part)");
    assert_equal_double(results, y, -5.0, "incorrect conversion to complex (imag part)");
}

void test_str_to(test_results *results)
{
    char32_t s[16];
    int64_t n;
    int error;

    str_to_char32(s, "45");
    n = str_to_int64(s, -100, 100, &error);
    assert_equal_int(results, error, 0, "str_to_int64 returned nonzero error");
    assert_equal_int64_t(results, n, 45, "str_to_int64 returned incorrect value");

    str_to_char32(s, "-97");
    n = str_to_int64(s, -100, 100, &error);
    assert_equal_int(results, error, 0, "str_to_int64 returned nonzero error");
    assert_equal_int64_t(results, n, -97, "str_to_int64 returned incorrect value");

    str_to_char32(s, "32767");
    n = str_to_int64(s, -32768, 32767, &error);
    assert_equal_int(results, error, 0, "str_to_int64 returned nonzero error");
    assert_equal_int64_t(results, n, 32767, "str_to_int64 returned incorrect value");

    str_to_char32(s, "-32768");
    n = str_to_int64(s, -32768, 32767, &error);
    assert_equal_int(results, error, 0, "str_to_int64 returned nonzero error");
    assert_equal_int64_t(results, n, -32768, "str_to_int64 returned incorrect value");

    str_to_char32(s, "32768");
    n = str_to_uint64(s, 100000UL, &error);
    assert_equal_int(results, error, 0, "str_to_uint64 returned nonzero error");
    assert_equal_uint64_t(results, n, 32768, "str_to_uint64 returned incorrect value");
}


void test_field_types(test_results *results)
{
    char *codes = "ffHHSU";
    int32_t sizes[] = {8, 8, 2, 2, 4, 48};
    int num_fields = sizeof(sizes) / sizeof(sizes[0]);

    field_type *ft = field_types_create(num_fields, codes, sizes);

    for (int k = 0; k < strlen(codes); ++k) {
        assert_equal_char(results, ft[k].typecode, codes[k], "ft[k].typecode not correct");
        assert_equal_int32_t(results, ft[k].itemsize, sizes[k], "ft[k].itemsize not correct");
    }    

    int status = field_types_grow(num_fields + 2, num_fields, &ft);
    ft[num_fields].typecode = 'b';
    ft[num_fields].itemsize = 1;
    ++num_fields;
    ft[num_fields].typecode = 'f';
    ft[num_fields].itemsize = 4;
    ++num_fields;

    int32_t total_size = field_types_total_size(num_fields, ft);
    assert_equal_int32_t(results, total_size, 77, "incorrect total size");

    bool homogeneous = field_types_is_homogeneous(num_fields, ft);
    assert_equal_bool(results, homogeneous, false, "homogeneous should be false");

    char *dtypestr = field_types_build_str(num_fields, NULL, false, ft);
    assert_equal_str(results, dtypestr, "f,f,H,H,S4,U12,b,f", "field_type_build_str() result not correct");
    free(dtypestr);

    free(ft);
}


void test_blocks(test_results *results)
{
    int row_size = 12;
    int rows_per_block = 4;
    int block_table_length = 3;

    int num_rows = 26;

    blocks_data *b = blocks_init(row_size, rows_per_block, block_table_length);
    if (b == NULL) {
        fprintf(stderr, "blocks_init returned NULL\n");
        exit(-1);
    }

    assert_equal_int(results, b->block_table_length, 3, "block_table_length not correct");

    for (size_t k = 0; k < num_rows; ++k) {
        char *ptr = blocks_get_row_ptr(b, k);
        if (ptr == NULL) {
            fprintf(stderr, "blocks_get_row_ptr(b, %zu) returned NULL\n", k);
            exit(-1);
        }
        memset(ptr, 'A' + k, row_size - 1);
        ptr[row_size-1] = '\0';
    }

    assert_equal_int(results, b->block_table_length, 12, "block_table_length not correct");

    for (size_t k = 0; k < b->block_table_length; ++k) {
        char *p = b->block_table[k];
        if (k > num_rows) {
            assert_equal_pointer(results, p, NULL, "pointer is not NULL");
        }
    }

    char *data = blocks_to_contiguous(b, num_rows);
    if (data == NULL) {
        fprintf(stderr, "blocks_to_contiguous returned NULL\n");
        exit(-1);
    }

    assert_equal_str(results, data, "AAAAAAAAAAA", "first record is not all 'A'");
    assert_equal_str(results, data + row_size*(num_rows - 1), "ZZZZZZZZZZZ", "last record is not all 'Z'");

    free(data);
    blocks_destroy(b);
}


void test_type_inference(test_results *results)
{
    char32_t s[16];
    char type, prev_type;
    int64_t i = 0;
    uint64_t u = 0;

    str_to_char32(s, "123");
    prev_type = '*';
    type = classify_type(s, '.', 'e', 'j', &i, &u, prev_type);
    assert_equal_char(results, type, 'Q', "inferred type is not 'Q'");
    assert_equal_uint64_t(results, u, 123, "value in u is not 123");

    str_to_char32(s, "1234");
    prev_type = 'd';
    type = classify_type(s, '.', 'e', 'j', &i, &u, prev_type);
    assert_equal_char(results, type, 'd', "inferred type is not 'd'");

    str_to_char32(s, "12X3");
    prev_type = 'd';
    type = classify_type(s, '.', 'e', 'j', &i, &u, prev_type);
    assert_equal_char(results, type, 'S', "inferred type is not 'S'");

    str_to_char32(s, "-12345");
    prev_type = '*';
    type = classify_type(s, '.', 'e', 'j', &i, &u, prev_type);
    assert_equal_char(results, type, 'q', "inferred type is not 'q'");
    assert_equal_int64_t(results, i, -12345, "value in u is not -12345");

    str_to_char32(s, "-12345");
    prev_type = 'Q';
    type = classify_type(s, '.', 'e', 'j', &i, &u, prev_type);
    assert_equal_char(results, type, 'q', "inferred type is not 'q'");
    assert_equal_int64_t(results, i, -12345, "value in u is not -12345");

    i = -10;
    u = 23;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'b', "type for range is not 'b'");

    i = 0;
    u = 23;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'B', "type for range is not 'B'");

    i = 0;
    u = 230;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'B', "type for range is not 'B'");

    i = 0;
    u = 500;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'H', "type for range is not 'H'");

    i = 0;
    u = 60000;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'H', "type for range is not 'H'");

    i = -100;
    u = 130;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'h', "type for range is not 'h'");

    i = 0;
    u = 65536;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'I', "type for range is not 'I'");

    i = -1;
    u = 65536;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'i', "type for range is not 'i'");

    i = 0;
    u = 1000000000000;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'Q', "type for range is not 'Q'");

    i = -1;
    u = 1000000000000;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'q', "type for range is not 'q'");

    i = -1000000000000;
    u = 0;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'q', "type for range is not 'q'");

    i = -1000000000000;
    u = 1000000000000;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'q', "type for range is not 'q'");

    i = -1000000000000;
    u = 9223372036854775808ULL;
    type = type_for_integer_range(i, u);
    assert_equal_char(results, type, 'd', "type for range is not 'd'");
}


int main(int argc, char *argv[])
{
    test_results results;
    int status;


    printf("### initializing\n");
    test_results_initialize(&results, "errlog-" __FILE__ ".out");

    printf("### running tests\n");

    printf("test_conversions\n");
    test_conversions(&results);

    printf("test_complex_conversion\n");
    test_complex_conversion(&results);

    printf("test_str_to\n");
    test_str_to(&results);

    printf("test_blocks\n");
    test_blocks(&results);

    printf("test_field_types\n");
    test_field_types(&results);

    printf("test_type_inference\n");
    test_type_inference(&results);

    printf("### finished running tests\n");

    test_results_print_summary(&results, __FILE__);
    test_results_finalize(&results);

    status = 0;
    if (results.num_failed > 0) {
        status = -1;
    }
    return status;
}
