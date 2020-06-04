
from os import path


def configuration(parent_package='', top_path=None):
    from numpy.distutils.misc_util import Configuration

    config = Configuration(None, parent_package, top_path)
    # config.add_data_dir('tests')
    config.add_subpackage('readtextstream')
    cfiles = ['_readtextmodule.c', 'analyze.c', 'type_inference.c',
              'rows.c', 'tokenize.c',
              'conversions.c', 'str_to.c', 'str_to_double.c', 'pow10table.c',
              'stream_file.c', 'stream_python_file_by_line.c', 'blocks.c',
              'field_types.c']
    config.add_extension('readtextstream._readtextmodule',
                         sources=[path.join('src', t) for t in cfiles])
    return config


if __name__ == '__main__':
    from numpy.distutils.core import setup
    setup(configuration=configuration)
