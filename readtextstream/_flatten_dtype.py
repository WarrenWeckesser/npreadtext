import numpy as np


_dtype_str_map = dict(i1='b', u1='B', i2='h', u2='H', i4='i', u4='I',
                      i8='q', u8='Q', f4='f', f8='d', c8='c', c16='z')

_dtype_str_map2 = dict(i1=('b', 1), u1=('B', 1),
                       i2=('h', 2), u2=('H', 2),
                       i4=('i', 4), u4=('I', 4),
                       i8=('q', 8), u8=('Q', 8),
                       f4=('f', 4), f8=('d', 8),
                       c8=('c', 8), c16=('z', 16))


def _dtypestr2fmt(st):
    """
    Convert a numpy dtype format string to an internal format string.

    Examples
    --------
    >>> _dtypestr2fmt('f8')
    'd'
    >>> _dtypestr2fmt('u4')
    'I'
    >>> _dtypestr2fmt('S10')
    '10s'
    """

    fmt = _dtype_str_map.get(st)
    if fmt is None:
        if st.startswith('U'):
            fmt = st[1:] + 'U'
        elif st.startswith('S'):
            fmt = st[1:] + 'S'
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


def _dtypestr2fmt2(st):
    """
    Convert dtype format string to an internal format character and size.

    Examples
    --------
    >>> _dtypestr2fmt2('f8')
    ('d', 8)
    >>> _dtypestr2fmt2('u4')
    ('I', 4)
    >>> _dtypestr2fmt2('S10')
    ('s', 10)
    """

    fmt = _dtype_str_map2.get(st)
    if fmt is None:
        if st.startswith('U'):
            fmt = ('U', 4*int(st[1:]))
        elif st.startswith('S'):
            fmt = ('s', int(st[1:]))
        else:
            raise ValueError('_dtypestr2fmt2: unsupported dtype string: %s' %
                             (st,))
    return fmt


def flatten_dtype2(dt):
    """
    Convert a numpy dtype into sequences of format codes and sizes.

    Examples
    --------
    XXX TODO: Update to show that return values are arrays...

    >>> codes, sizes = flatten_dtype2('float32,float64,int,uint16')
    >>> codes
    ['f', 'd', 'q', 'H']
    >>> sizes
    [4, 8, 8, 2]

    >>> dt = np.dtype([('name', 'S16'),
                       ('pos', [('x', np.uint16),
                                ('y', np.uint16)]),
                       ('color', [('r', np.uint8),
                                  ('g', np.uint8),
                                  ('b', np.uint8)]),
                       ('bar', float)])
    >>> codes, sizes = flatten_dtype2(dt)
    >>> codes
    ['s', 'H', 'H', 'B', 'B', 'B', 'd']
    >>> sizes
    [16, 2, 2, 1, 1, 1, 8]

    >>> dtb = np.dtype([('foo', [('q', np.int32), ('r', np.float32)], 3),
                        ('code', np.int16, 2)])
    >>> codes, sizes = flatten_dtype2(dtb)
    >>> codes
    ['i', 'f', 'i', 'f', 'i', 'f', 'h', 'h']
    >>> sizes
    [4, 4, 4, 4, 4, 4, 2, 2]
    """
    codes = []
    sizes = []
    _flatten_dtype2(dt, codes, sizes)
    # XXX Check convention for 's' vs 'S'
    codes = [c if c != 's' else 'S' for c in codes]
    return np.array(codes, dtype='S1'), np.array(sizes, dtype=np.int32)


def _flatten_dtype2(dt, codes, sizes):
    if not isinstance(dt, np.dtype):
        dt = np.dtype(dt)
    if dt.names is None:
        if dt.subdtype is not None:
            subdt, shape = dt.subdtype
            n = np.prod(shape)
            for k in range(n):
                _flatten_dtype2(subdt, codes, sizes)
        else:
            code, size = _dtypestr2fmt2(dt.str[1:])
            codes.append(code)
            sizes.append(size)
    else:
        for name in dt.names:
            _flatten_dtype2(dt[name], codes, sizes)
