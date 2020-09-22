
import argparse
import textwrap
import numpy as np
from npreadtext import read


def _loadtxt(*args, **kwds):
    delimiter = kwds.pop('delimiter', None)
    if delimiter is None:
        delimiter = ' '

    dtype = kwds.pop('dtype', None)
    if dtype is None:
        dtype = np.float64

    ndmin = kwds.pop('ndmin', None)
    if ndmin is None:
        ndmin = 0
    if ndmin not in [0, 1, 2]:
        raise ValueError(f'Illegal value of ndmin keyword: {ndmin}')

    comment = kwds.pop('comments', None)
    if comment is None:
        comment = ''
    elif isinstance(comment, bytes):
        comment = comment.decode('latin1')

    try:
        arr = read(*args, delimiter=delimiter, dtype=dtype,
                   comment=comment, **kwds)
    except RuntimeError as exc:
        raise ValueError(exc.args) from None

    # For now, ignore 'ndmin' if 'unpack' was given.
    if 'unpack' not in kwds:
        if arr.shape == (0, 0):
            arr.resize((0, 1))
        if ndmin == 2:
            if arr.shape == (0, 0):
                arr = arr.reshape((0, 1))
        elif ndmin == 1:
            arr = np.atleast_1d(np.squeeze(arr))
        else:
            arr = np.squeeze(arr)

    return arr


# Monkey patch numpy.loadtxt
np.loadtxt = _loadtxt


if __name__ == "__main__":
    descr = textwrap.dedent(
        """
        Run numpy tests with loadtxt replaced by a wrapper of the
        new reader.  Some useful tests to pass as the -t option:
            numpy.lib.tests.test_regression
            numpy.lib.tests.test_io::TestLoadTxt
        """)
    parser = argparse.ArgumentParser(
                description=descr,
                formatter_class=argparse.RawDescriptionHelpFormatter)

    test_help = ('Test to run, using the same syntax as the -t option '
                 'of runtests.py.')
    parser.add_argument('-t', '--test', required=True, help=test_help)

    verbose_help = ('Verbosity value for test outputs, in the range 1-3. '
                    'Default is 1.')
    parser.add_argument('-v', '--verbose', type=int, choices=[1, 2, 3],
                        default=1, help=verbose_help)

    args = parser.parse_args()

    np.test(verbose=args.verbose, tests=[args.test])
