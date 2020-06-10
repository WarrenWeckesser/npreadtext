/*
 *  This is a generated file.  Do not edit!
 */

#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include "ctestify.h"


void _assert_equal_bool(test_results *results,
                        bool value1, bool value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... bool values not equal: %d and %d\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_char(test_results *results,
                        char value1, char value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... char values not equal: %c and %c\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_int(test_results *results,
                        int value1, int value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... int values not equal: %d and %d\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_float(test_results *results,
                        float value1, float value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... float values not equal: %f and %f\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_close_float(test_results *results,
                        float value1, float value2, float abstol,
                        char *msg, char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (fabs(value1 - value2) > abstol) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... float values not close: %f and %f\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_double(test_results *results,
                        double value1, double value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... double values not equal: %g and %g\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_close_double(test_results *results,
                        double value1, double value2, double abstol,
                        char *msg, char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (fabs(value1 - value2) > abstol) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... double values not close: %g and %g\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_long(test_results *results,
                        long value1, long value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... long values not equal: %ld and %ld\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_longlong(test_results *results,
                        long long value1, long long value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... long long values not equal: %lld and %lld\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_int16_t(test_results *results,
                        int16_t value1, int16_t value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... int16_t values not equal: %" PRId16 " and %" PRId16 "\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_int32_t(test_results *results,
                        int32_t value1, int32_t value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... int32_t values not equal: %" PRId32 " and %" PRId32 "\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_int64_t(test_results *results,
                        int64_t value1, int64_t value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... int64_t values not equal: %" PRId64 " and %" PRId64 "\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_uint16_t(test_results *results,
                        uint16_t value1, uint16_t value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... uint16_t values not equal: %" PRIu16 " and %" PRIu16 "\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_uint32_t(test_results *results,
                        uint32_t value1, uint32_t value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... uint32_t values not equal: %" PRIu32 " and %" PRIu32 "\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}


void _assert_equal_uint64_t(test_results *results,
                        uint64_t value1, uint64_t value2, char *msg,
                        char *filename, int linenumber)
{
    results->num_assertions += 1;
    if (value1 != value2) {
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... uint64_t values not equal: %" PRIu64 " and %" PRIu64 "\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }
}

