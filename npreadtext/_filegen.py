
class FileGen:
    """
    Class that wraps a generator to make it look file-like.
    """
    def __init__(self, gen):
        self._gen = gen
        self._position = 0

    def readline(self):
        try:
            line = next(self._gen)
            self._position += len(line)
            line += '\n'
        except StopIteration:
            line = ''
        return line

    def seek(self, pos, whence=0):
        # For this class, seek(pos) is a no-op.
        return self._position

    def tell(self):
        return self._position
