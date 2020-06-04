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
#include "field_types.h"
#include "analyze.h"
#include "rows.h"
#include "error_types.h"


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

static void
raise_read_exception(read_error_type *read_error)
{
    if (read_error->error_type == 0) {
        // No error.
        return;
    }
    if (read_error->error_type == ERROR_OUT_OF_MEMORY) {
        PyErr_Format(PyExc_MemoryError, "out of memory while reading file");
    }
    else if (read_error->error_type == ERROR_INVALID_COLUMN_INDEX) {
        PyErr_Format(PyExc_RuntimeError, "line %d: invalid column index %d",
                     read_error->line_number, read_error->column_index);
    }
    else if (read_error->error_type == ERROR_BAD_FIELD) {
        PyErr_Format(PyExc_RuntimeError,
                     "line %d, field %d: bad %s value",
                     read_error->line_number, read_error->field_number + 1,
                     typecode_to_str(read_error->typecode));
    }
    else {
        // Some other error type
        PyErr_Format(PyExc_RuntimeError, "line %d: error type %d",
                     read_error->line_number, read_error->error_type);
    }
}

//
// Build a comma separated string representation of the dtype.
// If cols == NULL, it is not used.  Otherwise it is an array
// of length num_cols that gives the index into ft to use.
//
char *field_types_build_str(int num_cols, int32_t *cols, bool homogeneous, field_type *ft)
{
    char *dtypestr;
    size_t len;

    // Precompute the length of the string.
    // len = ...
    len = num_cols * 8;   // FIXME: crude estimate
    dtypestr = malloc(len);

    if (dtypestr == NULL) {
        return NULL;
    }

    // Fill in the string
    int p = 0;
    for (int j = 0; j < num_cols; ++j) {
        int k;
        if (cols == NULL) {
            // No indirection via cols.
            k = j;
        }
        else {
            // FIXME Values in usecols have not been validated!!!
            k = cols[j];
        }
        if (j > 0) {
            dtypestr[p++] = ',';
        }
        dtypestr[p++] = ft[j].typecode;
        // FIXME: What about typecode == 'U'?
        if (ft[j].typecode == 'S') {
            int nc = snprintf(dtypestr + p, len - p - 1, "%d", ft[j].itemsize);
            p += nc;
        }
        if (homogeneous) {
            break;
        }
    }
    dtypestr[p] = '\0';
    return dtypestr;
}


//
// `usecols` must point to a Python object that is Py_None or a 1-d contiguous
// numpy array with data type int32.
//
// `dtype` must point to a Python object that is Py_None or a numpy dtype
// instance.  If the latter, code and sizes must be arrays of length
// num_dtype_fields, holding the flattened data field type codes and byte
// sizes. (num_dtype_fields, codes, and sizes can be inferred from dtype,
// but we do that in Python code.)
//
// If both `usecols` and `dtype` are not None, and the data type is compound,
// then len(usecols) must equal num_dtype_fields.
//
// If `dtype` is given and it is compound, and `usecols` is None, then the
// number of columns in the file must match the number of fields in `dtype`.
//
static PyObject *
_readtext_from_stream(stream *s, char *filename, parser_config *pc,
                      PyObject *usecols, int skiprows, int max_rows,
                      PyObject *dtype, int num_dtype_fields, char *codes, int32_t *sizes)
{
    PyObject *arr = NULL;
    int32_t *cols;
    int ncols;
    npy_intp nrows;
    int num_fields;
    field_type *ft = NULL;
    size_t rowsize;

    bool homogeneous;
    npy_intp shape[2];

    if (dtype == Py_None) {
        // Make the first pass of the file to analyze the data type
        // and count the number of rows.
        // analyze() will assign num_fields and create the ft array,
        // based on the types of the data that it finds in the file.
        // XXX Note that analyze() does not use the usecols data--it
        // analyzes (and fills in ft for) all the columns in the file.
        nrows = analyze(s, pc, skiprows, -1, &num_fields, &ft);
        if (nrows < 0) {
            raise_analyze_exception(nrows, filename);
            return NULL;
        }
        stream_seek(s, 0);
        if (nrows == 0) {
            // Empty file, and a dtype was not given.  In this case, return
            // an array with shape (0, 0) and data type float64.
            npy_intp dims[2] = {0, 0};
            arr = PyArray_SimpleNew(2, dims, NPY_FLOAT64);
            return arr;
        }
    }
    else {
        // A dtype was given.
        num_fields = num_dtype_fields;
        ft = field_types_create(num_fields, codes, sizes);
        if (ft == NULL) {
            PyErr_Format(PyExc_MemoryError, "out of memory");
            return NULL;
        }
        nrows = max_rows;
    }

    homogeneous = field_types_is_homogeneous(num_fields, ft);
    rowsize = field_types_total_size(num_fields, ft);

    if (usecols == Py_None) {
        ncols = num_fields;
        cols = NULL;
    }
    else {
        ncols = PyArray_SIZE(usecols);
        cols = PyArray_DATA(usecols);
    }

    // XXX In the one-pass case, we don't have nrows.
    shape[0] = nrows;
    if (homogeneous) {
        shape[1] = ncols;
    }

    if (dtype == Py_None) {
        int num_cols;
        int ndim = homogeneous ? 2 : 1;

        // Build the dtype from the ft array of field types.
        char *dtypestr = field_types_build_str(ncols, cols, homogeneous, ft);
        if (dtypestr == NULL) {
            free(ft);
            PyErr_SetString(PyExc_MemoryError, "out of memory");
            return NULL;
        }

        PyObject *dtstr = PyUnicode_FromString(dtypestr);
        free(dtypestr);
        if (!dtstr) {
            free(ft);
            return NULL;
        }
        PyArray_Descr *dtype1;
        if (!PyArray_DescrConverter(dtstr, &dtype1)) {
            free(ft);
            return NULL;
        }

        arr = PyArray_SimpleNewFromDescr(ndim, shape, dtype1);
        if (!arr) {
            free(ft);
            return NULL;
        }
        read_error_type read_error;
        int num_rows = nrows;
        void *result = read_rows(s, &num_rows, num_fields, ft, pc,
                                 cols, ncols, skiprows, PyArray_DATA(arr),
                                 &num_cols, &read_error);
        if (read_error.error_type != 0) {
            free(ft);
            raise_read_exception(&read_error);
            return NULL;
        }
    }
    else {
        // A dtype was given.
        read_error_type read_error;
        int num_cols;
        int ndim;
        int num_rows = nrows;
        void *result = read_rows(s, &num_rows, num_fields, ft, pc,
                                 cols, ncols, skiprows, NULL,
                                 &num_cols, &read_error);
        if (read_error.error_type != 0) {
            free(ft);
            raise_read_exception(&read_error);
            return NULL;
        }

        shape[0] = num_rows;
        if (!PyDataType_ISEXTENDED(dtype)) {
            ndim = 2;
            if (num_rows > 0) {
                shape[1] = num_cols;
            }
            else {
                // num_rows == 0 => empty file.
                shape[1] = 0;
            }
        }
        else {
            ndim = 1;
            shape[1] = 1;  // Not actually necessary to fill this in.
        }
        // We have to INCREF dtype, because the Python caller owns a reference,
        // and PyArray_NewFromDescr steals a reference to it.
        Py_INCREF(dtype);
        // XXX Fix memory management - `result` was malloc'd.
        arr = PyArray_NewFromDescr(&PyArray_Type, (PyArray_Descr *) dtype,
                                   ndim, shape, NULL, result, 0, NULL);
        if (!arr) {
            free(ft);
            free(result);
            return NULL;
        }
    }

    free(ft);

    return arr;
}


static PyObject *
_readtext_from_filename(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"filename", "delimiter", "comment", "quote",
                             "decimal", "sci", "usecols", "skiprows",
                             "max_rows",
                             "dtype", "codes", "sizes",
                             "encoding", NULL};
    char *filename;
    char *delimiter = ",";
    char *comment = "#";
    char *quote = "\"";
    char *decimal = ".";
    char *sci = "E";
    int skiprows;
    int max_rows;

    PyObject *usecols;

    PyObject *dtype;
    PyObject *codes;
    PyObject *sizes;
    PyObject *encoding;

    char *codes_ptr = NULL;
    int32_t *sizes_ptr = NULL;

    parser_config pc;
    int buffer_size = 1 << 21;
    PyObject *arr = NULL;
    int num_dtype_fields;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|$sssssOiiOOOO", kwlist,
                                     &filename, &delimiter, &comment, &quote,
                                     &decimal, &sci, &usecols, &skiprows,
                                     &max_rows,
                                     &dtype, &codes, &sizes, &encoding)) {
        return NULL;
    }

    pc.delimiter = *delimiter;
    pc.comment = *comment;
    pc.quote = *quote;
    pc.decimal = *decimal;
    pc.sci = *sci;
    pc.allow_embedded_newline = true;
    pc.ignore_leading_spaces = false;
    pc.ignore_trailing_spaces = false;
    pc.ignore_blank_lines = true;
    pc.strict_num_fields = false;

    if (dtype == Py_None) {
        num_dtype_fields = -1;
        codes_ptr = NULL;
        sizes_ptr = NULL;
    }
    else {
        // If `dtype` is not None, then `codes` must be a contiguous 1-d numpy
        // array with dtype 'S1' (i.e. an array of characters), and `sizes`
        // must be a contiguous 1-d numpy array with dtype int32 that has the
        // same length as `codes`.  This code assumes this is true and does not
        // validate the arguments--it is expected that the calling code will
        // do so.
        num_dtype_fields = PyArray_SIZE(codes);
        codes_ptr = PyArray_DATA(codes);
        sizes_ptr = PyArray_DATA(sizes);
    }

    stream *s = stream_file_from_filename(filename, buffer_size);
    if (s == NULL) {
        PyErr_Format(PyExc_RuntimeError, "Unable to open '%s'", filename);
        return NULL;
    }

    arr = _readtext_from_stream(s, filename, &pc, usecols, skiprows, max_rows,
                                dtype, num_dtype_fields, codes_ptr, sizes_ptr);

    stream_close(s, RESTORE_NOT);
    return arr;
}


static PyObject *
_readtext_from_file_object(PyObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"file", "delimiter", "comment", "quote",
                             "decimal", "sci", "usecols", "skiprows",
                             "max_rows",
                             "dtype", "codes", "sizes",
                             "encoding", NULL};
    PyObject *file;
    char *delimiter = ",";
    char *comment = "#";
    char *quote = "\"";
    char *decimal = ".";
    char *sci = "E";
    int skiprows;
    int max_rows;
    PyObject *usecols;

    PyObject *dtype;
    PyObject *codes;
    PyObject *sizes;
    PyObject *encoding;

    char *codes_ptr = NULL;
    int32_t *sizes_ptr = NULL;

    parser_config pc;
    PyObject *arr = NULL;
    int num_dtype_fields;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|$sssssOiiOOOO", kwlist,
                                     &file, &delimiter, &comment, &quote,
                                     &decimal, &sci, &usecols, &skiprows,
                                     &max_rows,
                                     &dtype, &codes, &sizes, &encoding)) {
        return NULL;
    }

    pc.delimiter = *delimiter;
    pc.comment = *comment;
    pc.quote = *quote;
    pc.decimal = *decimal;
    pc.sci = *sci;
    pc.allow_embedded_newline = true;
    pc.ignore_leading_spaces = false;
    pc.ignore_trailing_spaces = false;
    pc.ignore_blank_lines = true;
    pc.strict_num_fields = false;

    if (dtype == Py_None) {
        num_dtype_fields = -1;
        codes_ptr = NULL;
        sizes_ptr = NULL;
    }
    else {
        // If `dtype` is not None, then `codes` must be a contiguous 1-d numpy
        // array with dtype 'S1' (i.e. an array of characters), and `sizes`
        // must be a contiguous 1-d numpy array with dtype int32 that has the
        // same length as `codes`.  This code assumes this is true and does not
        // validate the arguments--it is expected that the calling code will
        // do so.
        num_dtype_fields = PyArray_SIZE(codes);
        codes_ptr = PyArray_DATA(codes);
        sizes_ptr = PyArray_DATA(sizes);
    }

    stream *s = stream_python_file_by_line(file, encoding);
    if (s == NULL) {
        PyErr_Format(PyExc_RuntimeError, "Unable to access the file.");
        return NULL;
    }

    arr = _readtext_from_stream(s, NULL, &pc, usecols, skiprows, max_rows,
                                dtype, num_dtype_fields, codes_ptr, sizes_ptr);
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
