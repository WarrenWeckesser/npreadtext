

import pytest
import numpy as np
from numpy.testing import assert_array_equal
from readtextstream._flatten_dtype import flatten_dtype2


@pytest.mark.parametrize('dtype, expected_codes, expected_sizes', [
    (np.dtype(np.int32), [b'i'], [4]),
    (np.dtype(np.uint32), [b'I'], [4]),
    (np.dtype(np.float64), [b'd'], [8]),
    (np.dtype('f,f'), [b'f', b'f'], [4, 4]),
    (np.dtype([('x', np.int8), ('y', np.int8), ('name', 'S12')]),
     (b'b', b'b', b'S'), [1, 1, 12]),
    (np.dtype([('a', np.uint16, (2,)),
               ('b', np.uint16, (4,)),
               ('name', 'U64')]),
     (b'H', b'H', b'H', b'H', b'H', b'H', b'U'), [2, 2, 2, 2, 2, 2, 256]),
    (np.dtype('S'), [b'S'], [0]),
    (np.dtype('U'), [b'U'], [0]),
    (np.dtype('S1'), [b'S'], [1]),
    (np.dtype('U1'), [b'U'], [4]),
])
def test_flatten_dtype2(dtype, expected_codes, expected_sizes):
    codes, sizes = flatten_dtype2(dtype)
    assert_array_equal(codes, expected_codes)
    assert_array_equal(sizes, expected_sizes)
