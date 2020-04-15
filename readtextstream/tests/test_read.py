from os import path
import pytest
import numpy as np
from numpy.testing import assert_array_equal, assert_
from readtextstream import read


def _get_full_name(basename):
    return path.join(path.dirname(__file__), 'data', basename)


@pytest.mark.parametrize('basename,delim', [('test1.csv', ','),
                                            ('test1.tsv', '\t')])
def test1_read1(basename, delim):
    filename = _get_full_name(basename)
    al = np.loadtxt(filename, delimiter=delim)

    a = read(filename, delimiter=delim)
    assert_array_equal(a, al)

    with open(filename, 'r') as f:
        a = read(f, delimiter=delim)
        assert_array_equal(a, al)


@pytest.mark.parametrize('basename,sci', [('test1e.csv', 'E'),
                                          ('test1d.csv', 'D')])
def test1_read_sci(basename, sci):
    filename = path.join(path.dirname(__file__), 'data', basename)
    filename_e = path.join(path.dirname(__file__), 'data', 'test1e.csv')
    al = np.loadtxt(filename_e, delimiter=',')

    a = read(filename, sci=sci)
    assert_array_equal(a, al)

    with open(filename, 'r') as f:
        a = read(f, sci=sci)
        assert_array_equal(a, al)


@pytest.mark.parametrize('basename,delim', [('test1.csv', ','),
                                            ('test1.tsv', '\t')])
def test1_read_usecols(basename, delim):
    filename = path.join(path.dirname(__file__), 'data', basename)
    al = np.loadtxt(filename, delimiter=delim, usecols=[0, 2])

    a = read(filename, usecols=[0, 2], delimiter=delim)
    assert_array_equal(a, al)

    with open(filename, 'r') as f:
        a = read(f, usecols=[0, 2], delimiter=delim)
        assert_array_equal(a, al)


def test1_read_with_comment():
    filename = _get_full_name('test1_with_comments.csv')
    al = np.loadtxt(filename, delimiter=',')

    a = read(filename, comment='#')
    assert_array_equal(a, al)

    with open(filename, 'r') as f:
        a = read(f, comment='#')
        assert_array_equal(a, al)


def test_decimal_is_comma():
    filename = path.join(path.dirname(__file__),
                         'data', 'decimal_is_comma.txt')
    a = read(filename, decimal=',', delimiter=' ')
    expected = np.array([[1.5, 1.75, 2.0],
                         [2.5, 2.75, 3.0],
                         [3.5, 3.75, 4.0],
                         [4.5, 4.75, 5.0]])
    assert_array_equal(a, expected)


def test_quoted_field():
    filename = _get_full_name('quoted_field.csv')
    a = read(filename)
    expected_dtype = np.dtype([('f0', 'S8'), ('f1', np.float64)])
    assert_(a.dtype == expected_dtype)
    expected = np.array([('alpha, x', 2.5),
                         ('beta, x', 4.5),
                         ('gamma, x', 5.0)], dtype=expected_dtype)
    assert_array_equal(a, expected)


@pytest.mark.parametrize('explicit_dtype', [False, True])
@pytest.mark.parametrize('skiprows', [0, 1, 3])
def test_dtype_and_skiprows(explicit_dtype: bool, skiprows: int):
    filename = _get_full_name('mixed_types1.dat')

    expected_dtype = np.dtype([('f0', np.uint16),
                               ('f1', np.float64),
                               ('f2', 'S7'),
                               ('f3', np.int8)])
    expected = np.array([(1000, 2.4, "alpha", -34),
                         (2000, 3.1, "beta", 29),
                         (3500, 9.9, "gamma", 120),
                         (4090, 8.1, "delta", 0),
                         (5001, 4.4, "epsilon", -99),
                         (6543, 7.8, "omega", -1)], dtype=expected_dtype)

    dt = expected_dtype if explicit_dtype else None
    a = read(filename, quote="'", delimiter=';', skiprows=skiprows, dtype=dt)
    assert_array_equal(a, expected[skiprows:])


@pytest.mark.parametrize('param', ['skiprows', 'max_rows'])
@pytest.mark.parametrize('badval, exc', [(-3, ValueError), (1.0, TypeError)])
def test_bad_nonneg_int(param, badval, exc):
    with pytest.raises(exc):
        a = read('foo.bar', **{param: badval})
