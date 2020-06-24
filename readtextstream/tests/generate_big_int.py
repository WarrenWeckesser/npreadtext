
import sys
import numpy as np


rng = np.random.default_rng()

nrows = 2000000
ncols = 50
filename = 'data/bigint.csv'
print("Generating {}".format(filename))

with open(filename, 'w') as f:
    for k in range(nrows):
        values = rng.integers(1, 1000, size=ncols)
        s = ','.join(('%3d' % x) for x in values) + '\n'
        f.write(s)
        q, r = divmod(100*(k+1), nrows)
        if r == 0:
            print("\r{:3d}%".format(q), end='')
            sys.stdout.flush()
    print()
