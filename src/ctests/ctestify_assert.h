/*
 *  This is a generated file.  Do not edit!
 */

#ifndef CTESTIFY_ASSERT_H
#define CTESTIFY_ASSERT_H
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>

void _assert_equal_bool(test_results *results, bool value1, bool value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_bool(A, B, C, D) _assert_equal_bool(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_char(test_results *results, char value1, char value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_char(A, B, C, D) _assert_equal_char(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_int(test_results *results, int value1, int value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_int(A, B, C, D) _assert_equal_int(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_float(test_results *results, float value1, float value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_float(A, B, C, D) _assert_equal_float(A, B, C, D, __FILE__, __LINE__)

void _assert_close_float(test_results *results, float value1, float value2, float abstol,
                        char *msg, char *filename, int linenumber);
#define assert_close_float(A, B, C, D, E) _assert_close_float(A, B, C, D, E, __FILE__, __LINE__)

void _assert_equal_double(test_results *results, double value1, double value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_double(A, B, C, D) _assert_equal_double(A, B, C, D, __FILE__, __LINE__)

void _assert_close_double(test_results *results, double value1, double value2, double abstol,
                        char *msg, char *filename, int linenumber);
#define assert_close_double(A, B, C, D, E) _assert_close_double(A, B, C, D, E, __FILE__, __LINE__)

void _assert_equal_long(test_results *results, long value1, long value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_long(A, B, C, D) _assert_equal_long(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_longlong(test_results *results, long long value1, long long value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_longlong(A, B, C, D) _assert_equal_longlong(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_int16_t(test_results *results, int16_t value1, int16_t value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_int16_t(A, B, C, D) _assert_equal_int16_t(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_int32_t(test_results *results, int32_t value1, int32_t value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_int32_t(A, B, C, D) _assert_equal_int32_t(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_int64_t(test_results *results, int64_t value1, int64_t value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_int64_t(A, B, C, D) _assert_equal_int64_t(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_uint16_t(test_results *results, uint16_t value1, uint16_t value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_uint16_t(A, B, C, D) _assert_equal_uint16_t(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_uint32_t(test_results *results, uint32_t value1, uint32_t value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_uint32_t(A, B, C, D) _assert_equal_uint32_t(A, B, C, D, __FILE__, __LINE__)

void _assert_equal_uint64_t(test_results *results, uint64_t value1, uint64_t value2, char *msg,
                        char *filename, int linenumber);
#define assert_equal_uint64_t(A, B, C, D) _assert_equal_uint64_t(A, B, C, D, __FILE__, __LINE__)


#endif

