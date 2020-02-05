#ifndef STREAM_FILE_H
#define STREAM_FILE_H

#include "stream.h"

stream *stream_file(FILE *f, int buffer_size);
stream *stream_file_from_filename(char *filename, int buffer_size);

#endif
