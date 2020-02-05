#ifndef _TYPE_INFERENCE_H_
#define _TYPE_INFERENCE_H_

char classify_type(char *field, char decimal, char sci,
                   int64_t *i, uint64_t *u,
                   //char *datetime_fmt,
                   char prev_type);
char type_for_integer_range(int64_t imin, uint64_t umax);

#endif
