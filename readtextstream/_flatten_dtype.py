import numpy as np


_dtype_str_map = dict(i1='b', u1='B', i2='h', u2='H', i4='i', u4='I',
                      i8='q', u8='Q', f4='f', f8='d', c8='c', c16='z')


def _dtypestr2fmt(st):
    """
    Convert a numpy dtype format string to an internal format string.

    Examples
    --------
    >>> _dtypestr2fmt('f8')
    'd'
    >>> _dtypestr2fmt('u4')
    'I'
    >>> dtypestr2fmt('S10')
    '10s'
    """

    fmt = _dtype_str_map.get(st)
    if fmt is None:
        if st == "M8[us]":
            fmt = 'U'  # Temporary experiment
        elif st.startswith('S'):
            fmt = st[1:] + 's'
        else:
            raise ValueError('_dtypestr2fmt: unsupported dtype string: %s' %
                             (st,))
    return fmt


def _prod(x, y):
    return x*y


def flatten_dtype(dt):
    """
    Convert a numpy dtype into a format string.

    Examples
    --------
    >>> flatten_dtype('float32,float64,int,uint16')
    'fdiH'
    >>> dt = np.dtype([('name', 'S16'),
                       ('pos', [('x', np.uint16),
                                ('y', np.uint16)]),
                       ('color', [('r', np.uint8),
                                  ('g', np.uint8),
                                  ('b', np.uint8)]),
                       ('bar', float)])
    >>> flatten_dtype(dt)
    '16sHHBBBd'
    """
    if not isinstance(dt, np.dtype):
        dt = np.dtype(dt)
    if dt.names is None:
        if dt.subdtype is not None:
            subdt, shape = dt.subdtype
            n = np.prod(shape)
            fmt = flatten_dtype(subdt) * n
        else:
            fmt = _dtypestr2fmt(dt.str[1:])
    else:
        fmt = ''.join([flatten_dtype(dt[name]) for name in dt.names])
    return fmt
