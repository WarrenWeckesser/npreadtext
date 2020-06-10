#ifndef CTESTIFY_H
#define CTESTIFY_H

#include <stdio.h>

typedef struct _test_results {
    int num_assertions;
    int num_failed;
    char *errfilename;
    FILE *errfile;
} test_results;

void test_results_initialize(test_results *results, char *errfilename);
void test_results_finalize(test_results *results);
void test_results_fprint_summary(FILE *out, test_results *results, char *label);
void test_results_print_summary(test_results *results, char *label);

void _assert_true(test_results *results, int value, char *msg, char *filename,
				 int linenumber);
void _assert_equal_pointer(test_results *results, void *value1, void *value2,
                          char *msg, char *filename, int linenumber);
void _assert_equal_str(test_results *results, char *value1, char *value2,
                      char *msg, char *filename, int linenumber);

#include "ctestify_assert.h"

#define assert_true(A, B, C) _assert_true(A, B, C, __FILE__, __LINE__)
#define assert_equal_pointer(A, B, C, D) _assert_equal_pointer(A, B, C, D, __FILE__, __LINE__)
#define assert_equal_str(A, B, C, D) _assert_equal_str(A, B, C, D, __FILE__, __LINE__)


#endif
