

template = r"""
void _assert_equal_{type_nosp}(test_results *results,
                        {type} value1, {type} value2, char *msg,
                        char *filename, int linenumber)
{{
    results->num_assertions += 1;
    if (value1 != value2) {{
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... {type} values not equal: {format} and {format}\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }}
}}
"""

close_template = r"""
void _assert_close_{type_nosp}(test_results *results,
                        {type} value1, {type} value2, {type} abstol,
                        char *msg, char *filename, int linenumber)
{{
    results->num_assertions += 1;
    if (fabs(value1 - value2) > abstol) {{
        fprintf(results->errfile, "Assertion failed: %s:%d  %s\n", filename, linenumber, msg);
        fprintf(results->errfile, "... {type} values not close: {format} and {format}\n", value1, value2);
        fflush(results->errfile);
        results->num_failed += 1;
    }}
}}
"""

types = [
    ('bool', '%d'),
    ('char', '%c'),
    ('int', '%d'),
    ('float', '%f'),
    ('double', '%g'),
    ('long', '%ld'),
    ('long long', '%lld'),
    ('int16_t', '%" PRId16 "'),
    ('int32_t', '%" PRId32 "'),
    ('int64_t', '%" PRId64 "'),
    ('uint16_t', '%" PRIu16 "'),
    ('uint32_t', '%" PRIu32 "'),
    ('uint64_t', '%" PRIu64 "'),
]

cfilename = 'ctestify_assert.c'
hfilename = 'ctestify_assert.h'
cfile = open(cfilename, 'w')
hfile = open(hfilename, 'w')

gen_msg = "/*\n *  This is a generated file.  Do not edit!\n */\n"
cfile.write(gen_msg)
hfile.write(gen_msg)

initial_c = """
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <math.h>
#include "ctestify.h"
"""

initial_h = """
#ifndef CTESTIFY_ASSERT_H
#define CTESTIFY_ASSERT_H
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
"""

final_h = """
#endif
"""

print(initial_c, file=cfile)
print(initial_h, file=hfile)

for tp, fmt in types:
    tp_nosp = tp.replace(' ', '')
    func = template.format(type=tp, type_nosp=tp_nosp, format=fmt)
    print(func, file=cfile)
    line1, line2, line3 = func.lstrip().splitlines()[:3]
    sig = '\n'.join([line1, line2, line3.rstrip()]) + ';'
    print(sig, file=hfile)
    macro = ("#define assert_equal_{type_nosp}(A, B, C, D) "
             "_assert_equal_{type_nosp}(A, B, C, D, __FILE__, __LINE__)"
             .format(type_nosp=tp_nosp))
    print(macro, file=hfile)
    print(file=hfile)

    if tp in ['float', 'double']:
        func = close_template.format(type=tp, type_nosp=tp_nosp, format=fmt)
        print(func, file=cfile)
        line1, line2, line3 = func.lstrip().splitlines()[:3]
        sig = '\n'.join([line1, line2, line3.rstrip()]) + ';'
        print(sig, file=hfile)
        macro = ("#define assert_close_{type_nosp}(A, B, C, D, E) "
                 "_assert_close_{type_nosp}(A, B, C, D, E, __FILE__, __LINE__)"
                 .format(type_nosp=tp_nosp))
        print(macro, file=hfile)
        print(file=hfile)


print(final_h, file=hfile)

cfile.close()
hfile.close()

print("Generated {} and {}".format(cfilename, hfilename))
