//
//  Requires C99.
//

#include <stdio.h>
#include <stdbool.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <numpy/arrayobject.h>

#include "parser_config.h"
#include "stream_file.h"
#include "stream_python_file_by_line.h"
#include "field_type.h"
#include "analyze.h"
#include "rows.h"

// FIXME: This hard-coded constant should not be necessary.
#define DTYPESTR_SIZE 100

static void
raise_analyze_exception(int nrows, char *filename);


static PyObject *
_readtext_from_stream(stream *s, char *filename, parser_config *pc,
                      PyObject *usecols)
{
    PyObject *arr = NULL;
    int32_t *cols;
    int ncols;
    npy_intp nrows;
    int num_fields;
    field_type *ft;
    size_t rowsize;

    bool homogeneous;
    npy_intp shape[2];

    char dtypestr[DTYPESTR_SIZE];

    nrows = analyze(s, pc, 0, -1, &num_fields, &ft);
    if (nrows < 0) {
        raise_analyze_exception(nrows, filename);
        return NULL;
    }

    stream_seek(s, 0);  // XXX FIXME

    // Check if all the fields are the same type.
    homogeneous = true;
    rowsize = ft[0].itemsize;
    for (int k = 1; k < num_fields; ++k) {
        rowsize += ft[k].itemsize;
        if ((ft[k].typecode != ft[0].typecode) || (ft[k].itemsize != ft[0].itemsize)) {
            homogeneous = false;
            //break;
        }
    }

    if (usecols == Py_None) {
        ncols = num_fields;
        cols = NULL;
    }
    else {
        ncols = PyArray_SIZE(usecols);
        cols = PyArray_DATA(usecols);
    }

    shape[0] = nrows;
    if (homogeneous) {
        shape[1] = ncols;
    }

    // Build a comma separated string representation
    // of the dtype.
    // FIXME: make this a separate function.
    int p = 0;
    for (int j = 0; j < ncols; ++j) {
        int k;
        if (usecols == Py_None) {
            k = j;
        }
        else {
            // FIXME Values in usecols have not been validated!!!
            k = cols[j];
        }
        if (j > 0) {
            dtypestr[p++] = ',';
        }
        dtypestr[p++] = ft[k].typecode;
        if (ft[k].typecode == 'S') {
            int nc = snprintf(dtypestr + p, DTYPESTR_SIZE - p - 1, "%d", ft[k].itemsize);
            p += nc;
        }
        if (homogeneous) {
            break;
        }
    }
    dtypestr[p] = 0;

    PyObject *dtstr = PyUnicode_FromString(dtypestr);
    PyArray_Descr *dtype;
    int check = PyArray_DescrConverter(dtstr, &dtype);
    // FIXME: Handle !check.
    if (check) {
        int ndim = homogeneous ? 2 : 1;
        // FIXME: Handle failure here...
        arr = PyArray_SimpleNewFromDescr(ndim, shape, dtype);
        if (arr) {
            // XXX when the following is false, some unit tests
            // pass, but the test of usecols crashes or hangs python.
            if (true) {
                int num_rows = nrows;
                int error_type;
                int error_lineno;
                void *result = read_rows(s, &num_rows, num_fields, ft, pc,
                                         cols, ncols, 0, PyArray_DATA(arr),
                                         &error_type, &error_lineno);
            }
            else {
                //int num_rows = nrows;
                int num_rows = -1;
                int error_type;
                int error_lineno;
                void *result = read_rows(s, &num_rows, num_fields, ft, pc,
                                         cols, ncols, 0, NULL,
                                         &error_type, &error_lineno);
                // FIXME: This copy should not be necessary (hacking for now)...
                memcpy(PyArray_DATA(arr), result, rowsize * num_rows);
            }
        }
    }
    free(ft);

    return arr;
}

static void
raise_analyze_exception(int nrows, char *filename)
{
    if (nrows == ANALYZE_OUT_OF_MEMORY) {
        if (filename) {
            PyErr_Format(PyExc_MemoryError,
                         "Out of memory while analyzing '%s'", filename);
        } else {
            PyErr_Format(PyExc_MemoryError,
                         "Out of memory while analyzing file.");
        }
    }
    else if (nrows == ANALYZE_FILE_ERROR) {
        if (filename) {
            PyErr_Format(PyExc_RuntimeError,
                         "File error while analyzing '%s'", filename);
        } else {
            PyErr_Format(PyExc_RuntimeError,
                         "File error while analyzing file.");
        }
    }
    else {
        if (filename) {
            PyErr_Format(PyExc_RuntimeError,
                         "Unknown error when analyzing '%s'", filename);
        } else {
            PyErr_Format(PyExc_RuntimeError,
                         "Unknown error when analyzing file.");
        }
    }
}

static PyObject *
_readtext_from_filename(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"filename", "delimiter", "comment", "quote",
                             "decimal", "sci", "usecols", NULL};
    char *filename;
    char *delimiter = ",";
    char *comment = "#";
    char *quote = "\"";
    char *decimal = ".";
    char *sci = "E";
    PyObject *usecols;
    parser_config pc;
    int buffer_size = 1 << 21;
    PyObject *arr = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|$sssssO", kwlist,
                                     &filename, &delimiter, &comment, &quote,
                                     &decimal, &sci, &usecols)) {
        return NULL;
    }

    pc.delimiter = *delimiter;
    pc.comment = *comment;
    pc.quote = *quote;
    pc.decimal = *decimal;
    pc.sci = *sci;
    pc.allow_embedded_newline = true;
    pc.ignore_leading_spaces = true;
    pc.ignore_trailing_spaces = true;
    pc.strict_num_fields = false;

    stream *s = stream_file_from_filename(filename, buffer_size);
    if (s == NULL) {
        PyErr_Format(PyExc_RuntimeError, "Unable to open '%s'", filename);
        return NULL;
    }
    arr = _readtext_from_stream(s, filename, &pc, usecols);
    stream_close(s, RESTORE_NOT);
    return arr;
}


static PyObject *
_readtext_from_file_object(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"file", "delimiter", "comment", "quote",
                             "decimal", "sci", "usecols", NULL};
    PyObject *file;
    char *delimiter = ",";
    char *comment = "#";
    char *quote = "\"";
    char *decimal = ".";
    char *sci = "E";
    PyObject *usecols;
    parser_config pc;
    //int buffer_size = 1 << 21;
    PyObject *arr = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|$sssssO", kwlist,
                                     &file, &delimiter, &comment, &quote,
                                     &decimal, &sci, &usecols)) {
        return NULL;
    }

    pc.delimiter = *delimiter;
    pc.comment = *comment;
    pc.quote = *quote;
    pc.decimal = *decimal;
    pc.sci = *sci;
    pc.allow_embedded_newline = true;
    pc.ignore_leading_spaces = true;
    pc.ignore_trailing_spaces = true;
    pc.strict_num_fields = false;

    stream *s = stream_python_file_by_line(file);
    if (s == NULL) {
        PyErr_Format(PyExc_RuntimeError, "Unable to access the file.");
        return NULL;
    }
    arr = _readtext_from_stream(s, NULL, &pc, usecols);
    stream_close(s, RESTORE_NOT);
    return arr;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Python extension module definition.
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PyMethodDef module_methods[] = {
    {"_readtext_from_file_object", (PyCFunction) _readtext_from_file_object,
         METH_VARARGS | METH_KEYWORDS, "testing"},
    {"_readtext_from_filename", (PyCFunction) _readtext_from_filename,
         METH_VARARGS | METH_KEYWORDS, "testing"},
    {0} // sentinel
};

static struct PyModuleDef moduledef = {
    .m_base     = PyModuleDef_HEAD_INIT,
    .m_name     = "_readtextmodule",
    .m_size     = -1,
    .m_methods  = module_methods,
};


PyMODINIT_FUNC
PyInit__readtextmodule(void)
{
    PyObject* m = NULL;

    //
    // Initialize numpy.
    //
    import_array();
    if (PyErr_Occurred()) {
        return NULL;
    }

    // ----------------------------------------------------------------
    // Finish the extension module creation.
    // ----------------------------------------------------------------  

    // Create module
    m = PyModule_Create(&moduledef);

    return m;
}
