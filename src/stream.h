#ifndef _STREAM_H_
#define _STREAM_H_

#include <stdint.h>

#define STREAM_EOF   -1
#define STREAM_ERROR -2

#define RESTORE_NOT     0
#define RESTORE_INITIAL 1
#define RESTORE_FINAL   2


typedef struct _stream {
    void *stream_data;
    int32_t (*stream_fetch)(void *sdata);
    int32_t (*stream_peek)(void *sdata);
    void (*stream_skipline)(void *sdata);
    void (*stream_skiplines)(void *sdata, int n);
    int (*stream_linenumber)(void *sdata);
    int (*stream_lineoffset)(void *sdata);
    long int (*stream_tell)(void *sdata);
    int (*stream_seek)(void *sdata, long int pos);
    // Note that the first argument to stream_close is the stream pointer
    // itself, not the stream_data pointer.
    int (*stream_close)(void *strm, int);
} stream;


#define stream_fetch(s)             ((s)->stream_fetch((s)->stream_data))
#define stream_peek(s)              ((s)->stream_peek((s)->stream_data))
#define stream_skipline(s)          ((s)->stream_skipline((s)->stream_data))
#define stream_skiplines(s, n)      ((s)->stream_skiplines((s)->stream_data, (n)))
#define stream_linenumber(s)        ((s)->stream_linenumber((s)->stream_data))
#define stream_lineoffset(s)        ((s)->stream_lineoffset((s)->stream_data))
#define stream_seek(s, pos)         ((s)->stream_seek((s)->stream_data, (pos)))
#define stream_tell(s)              ((s)->stream_tell((s)->stream_data))
#define stream_close(s, restore)    ((s)->stream_close((s), (restore)))

#endif
