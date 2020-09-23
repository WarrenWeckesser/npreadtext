
from os import path


def get_version():
    """
    Find the value assigned to __version__ in npreadtext/__init__.py.

    This function assumes that there is a line of the form

        __version__ = "version-string"

    in npreadtext/__init__.py.  It returns the string version-string, or None
    if such a line is not found.
    """
    with open(path.join('npreadtext', '__init__.py'), 'r') as f:
        for line in f:
            s = [w.strip() for w in line.split('=', 1)]
            if len(s) == 2 and s[0] == '__version__':
                return s[1][1:-1]


_descr = ('Read text files into a NumPy array.')

with open('README.rst', 'r') as f:
    _long_descr = f.read()


def configuration(parent_package='', top_path=None):
    from numpy.distutils.misc_util import Configuration

    config = Configuration(None, parent_package, top_path)
    # config.add_data_dir('tests')
    config.add_subpackage('npreadtext')
    cfiles = ['_readtextmodule.c', 'analyze.c', 'type_inference.c',
              'rows.c', 'tokenize.c',
              'conversions.c', 'str_to.c', 'str_to_int.c', 'str_to_double.c',
              'pow10table.c',
              'stream_file.c', 'stream_python_file_by_line.c', 'blocks.c',
              'char32utils.c', 'field_types.c', 'dtoa_modified.c']
    config.add_extension('npreadtext._readtextmodule',
                         sources=[path.join('src', t) for t in cfiles])
    return config


if __name__ == '__main__':
    from numpy.distutils.core import setup
    setup(
        name='npreadtext',
        configuration=configuration,
        author='Warren Weckesser',
        version=get_version(),
        description=_descr,
        long_description=_long_descr,
        classifiers=["Programming Language :: Python :: 3",
                     "License :: OSI Approved :: BSD License"],
    )
