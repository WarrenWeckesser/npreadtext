//
// stream_python_file_by_line.c
//
// The public function defined in this file is
//
//     stream *stream_python_file_by_line(PyObject *obj)
//
// This function wraps a Python file object in a stream that
// can be used by the text file reader.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include <sys/types.h>
#include <unistd.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "stream.h"


typedef struct _python_file_by_line {

    /* The Python file object being read. */
    PyObject *file;

    /* The `readline` attribute of the file object. */
    PyObject *readline;

    /* The `seek` attribute of the file object. */
    PyObject *seek;

    /* The `tell` attribute of the file object. */
    PyObject *tell;

    /* file position when the file_buffer was created. */
    off_t initial_file_pos;

    int32_t line_number;

    /* Boolean: has the end of the file been reached? */
    int reached_eof;

    /* Python str object holding the line most recently read from the file. */
    PyObject *line;

    /* Length of line */
    Py_ssize_t linelen;

    /* Unicode kind of line (see the Python docs about unicode) */
    int unicode_kind;

    /* The DATA associated with line. */
    void *unicode_data;

    /* An empty Python tuple. This is the argument passed to readline. */
    PyObject *empty_tuple;

    /* Position in the buffer of the next character to read. */
    Py_ssize_t current_buffer_pos;

    // encoding must be None or a bytes object holding an
    // ASCII-encoding string, e.g. b'utf-8'.
    PyObject *encoding;

} python_file_by_line;

#define FB(fb)  ((python_file_by_line *)fb)

static
int32_t fb_line_number(void *fb)
{
    return FB(fb)->line_number;
}

/*
 *  int _fb_load(void *fb)
 *
 *  Get data from the file into the buffer.
 *
 *  Returns 0 on success.
 *  Returns STREAM_ERROR on error.
 */

static
int _fb_load(void *fb)
{
    //printf("_fb_load: starting\n");
    //printf("FB(fb)->reached_eof = %d\n", FB(fb)->reached_eof);
    //printf("FB(fb)->current_buffer_pos = %ld\n", FB(fb)->current_buffer_pos);
    //printf("FB(fb)->linelen = %ld\n", FB(fb)->linelen);

    if (!FB(fb)->reached_eof && (FB(fb)->current_buffer_pos == FB(fb)->linelen)) {
        // Read a line from the file.
        //printf("_fb_load: calling readline\n");
        PyObject *line = PyObject_Call(FB(fb)->readline, FB(fb)->empty_tuple, NULL);
        //printf("_fb_load: back from readline\n");
        FB(fb)->line = line;
        if (line == NULL) {
            //printf("_fb_load: STREAM_ERROR\n");
            return STREAM_ERROR;
        }
        Py_INCREF(line);
        if (PyBytes_Check(line)) {
            PyObject *uline;
            char *enc;
            // readline() returned bytes, so encode it.
            // XXX if no encoding was specified, assume UTF-8.
            if (FB(fb)->encoding == Py_None) {
                enc = "utf-8";
            }
            else {
                enc = PyBytes_AsString(FB(fb)->encoding);
            }
            uline = PyUnicode_FromEncodedObject(line, enc, NULL);
            if (uline == NULL) {
                Py_DECREF(line);
                // XXX temporary printf
                printf("_fb_load: failed to decode bytes object\n");
                return STREAM_ERROR;
            }
            Py_INCREF(uline);
            Py_DECREF(line);
            line = uline;
        }

        // Cache data about the line in the fb object.
        FB(fb)->unicode_kind = PyUnicode_KIND(line);
        FB(fb)->unicode_data = PyUnicode_DATA(line);
        FB(fb)->linelen = PyUnicode_GET_LENGTH(line);

        // Reset the character position to 0.
        FB(fb)->current_buffer_pos = 0;

        // If readline() returned 0, we've reached the end of the file.
        if (FB(fb)->linelen == 0) {
            FB(fb)->reached_eof = true;
        }
    }
    //printf("_fb_load: returning 0\n");
    return 0;
}

/*
 *  int32_t fb_fetch(file_buffer *fb)
 *
 *  Get a single character from the current line, and advance the
 *  buffer pointer.
 *
 *  Returns STREAM_EOF when the end of the file is reached.
 *  XXX The following comments are probably no longer correct... XXX
 *  The sequence '\r\n' is treated as a single '\n'.  That is, when the next
 *  two bytes in the buffer are '\r\n', the buffer pointer is advanced by 2
 *  and '\n' is returned.
 *  When '\n' is returned, fb->line_number is incremented.
 */

static
int32_t fb_fetch(void *fb)
{
    int32_t c;
  
    //printf("fb_fetch: starting, fb = %lld\n", fb);

    if (_fb_load(fb) != 0) {
        return STREAM_ERROR;
    }
    if (FB(fb)->reached_eof) {
        return STREAM_EOF;
    }

    c = (int32_t) PyUnicode_READ(FB(fb)->unicode_kind,
                                 FB(fb)->unicode_data,
                                 FB(fb)->current_buffer_pos);
    FB(fb)->current_buffer_pos++;
    if (c == '\n') {
        FB(fb)->line_number++;
    }
    return c;
}


/*
 *  int32_t fb_next(file_buffer *fb)
 *
 *  Returns the next byte in the buffer, but does not advance the pointer.
 *  If the next two characters in the buffer are "\r\n", '\n' is returned.
 */

static
int32_t fb_next(void *fb)
{
    int32_t c;

    if (_fb_load(fb) != 0) {
        return STREAM_ERROR;
    }
    if (FB(fb)->reached_eof) {
        return STREAM_EOF;
    }

    c = (int32_t) PyUnicode_READ(FB(fb)->unicode_kind,
                                 FB(fb)->unicode_data,
                                 FB(fb)->current_buffer_pos);
    return c;
}

/*
 *  fb_skipline(void *fb)
 *
 *  Read bytes from the buffer until a newline or the end of the file is reached.
 *
 *  The return value is 0 if no errors occurred.
 */

static
int32_t fb_skipline(void *fb)
{
    int32_t c;

    c = fb_next(fb);
    if (c == STREAM_ERROR) {
        return c;
    }
    while (c != '\n' && c != STREAM_EOF) {
        fb_fetch(fb);
        c = fb_next(fb);
        if (c == STREAM_ERROR) {
            return c;
        }
    }
    if (c == '\n') {
        fb_fetch(fb);
    }
    return 0;
}


/*
 *  fb_skiplines(void *fb, int num_lines)
 *
 *  Skip num_lines; calls fb_skipline(fb) num_lines times, or until the end of
 *  the file is reached.
 *
 *  The return value is 0 if no errors occurred.
 */

static
int32_t fb_skiplines(void *fb, int num_lines)
{
    int32_t status;
    int32_t c;

    while (num_lines > 0) {
        status = fb_skipline(fb);
        if (status != 0) {
            return status;
        }
        c = fb_next(fb);
        if (c == STREAM_EOF) {
            break;
        }
        if (c < 0) {
            // Error
            return c;
        }
        --num_lines;
    }
    return 0;
}

// XXX Is long int really the correct type?
static
long int fb_tell(void *fb)
{
    long int pos;
    PyObject *obj = PyObject_Call(FB(fb)->tell, FB(fb)->empty_tuple, NULL);
    // XXX Check for error.
    pos = PyLong_AsLong(obj);
    return pos;
}

static
int fb_seek(void *fb, long int pos)
{
    int status = 0;

    PyObject *args = Py_BuildValue("(n)", (Py_ssize_t) pos);
    // XXX Check for error, and
    // DECREF where appropriate...
    PyObject *result = PyObject_Call(FB(fb)->seek, args, NULL);
    // XXX Check for error!
    FB(fb)->line_number = 1;
    //FB(fb)->buffer_file_pos = FB(fb)->initial_file_pos;
    FB(fb)->current_buffer_pos = 0;
    //FB(fb)->last_pos = 0;
    FB(fb)->reached_eof = false;
    return status;
}

static
int stream_del(stream *strm, int restore)
{
    python_file_by_line *fb = (python_file_by_line *) (strm->stream_data);

    if (restore == RESTORE_INITIAL) {
        // XXX
        stream_seek(strm, SEEK_SET);
        //fseek(FB(fb)->file, FB(fb)->initial_file_pos, SEEK_SET);
    }
    else if (restore == RESTORE_FINAL) {
        // XXX
        stream_seek(strm, SEEK_SET);
        //fseek(FB(fb)->file, FB(fb)->buffer_file_pos + FB(fb)->current_buffer_pos, SEEK_SET);
    }

    // XXX Wrap the following clean up code in something more modular?
    Py_XDECREF(fb->file);
    Py_XDECREF(fb->readline);
    Py_XDECREF(fb->seek);
    Py_XDECREF(fb->tell);
    Py_XDECREF(fb->empty_tuple);

    free(fb);
    free(strm);

    return 0;
}


stream *stream_python_file_by_line(PyObject *obj, PyObject *encoding)
{
    python_file_by_line *fb;
    stream *strm;
    PyObject *func;

    fb = (python_file_by_line *) malloc(sizeof(python_file_by_line));
    //printf("fb = %lld\n", fb);
    if (fb == NULL) {
        // XXX handle the errors here and below properly.
        fprintf(stderr, "stream_file: malloc() failed.\n");
        return NULL;
    }

    fb->file = NULL;
    fb->readline = NULL;
    fb->seek = NULL;
    fb->tell = NULL;
    fb->empty_tuple = NULL;
    fb->encoding = encoding;

    strm = (stream *) malloc(sizeof(stream));
    if (strm == NULL) {
        fprintf(stderr, "stream_file: malloc() failed.\n");
        free(fb);
        return NULL;
    }

    fb->file = obj;
    Py_INCREF(fb->file);

    func = PyObject_GetAttrString(obj, "readline");
    if (!func) {
        goto fail;
    }
    fb->readline = func;
    Py_INCREF(fb->readline);

    func = PyObject_GetAttrString(obj, "seek");
    if (!func) {
        goto fail;
    }
    fb->seek = func;
    Py_INCREF(fb->seek);

    func = PyObject_GetAttrString(obj, "tell");
    if (!func) {
        goto fail;
    }
    fb->tell = func;
    Py_INCREF(fb->tell);

    fb->empty_tuple = PyTuple_New(0);
    if (fb->empty_tuple == NULL) {
        goto fail;
    }

    fb->line_number = 1;
    fb->linelen = 0;

    fb->current_buffer_pos = 0;
    //fb->last_pos = 0;
    fb->reached_eof = 0;

    strm->stream_data = (void *) fb;
    strm->stream_fetch = &fb_fetch;
    strm->stream_peek = &fb_next;
    strm->stream_skipline = &fb_skipline;
    strm->stream_skiplines = &fb_skiplines;
    strm->stream_linenumber = &fb_line_number;
    strm->stream_tell = &fb_tell;
    strm->stream_seek = &fb_seek;
    strm->stream_close = &stream_del;  // FIXME: compiler warning

    return strm;

fail:
    Py_XDECREF(fb->file);
    Py_XDECREF(fb->readline);
    Py_XDECREF(fb->seek);
    Py_XDECREF(fb->tell);
    Py_XDECREF(fb->empty_tuple);

    free(fb);
    free(strm);
    return NULL;
}
