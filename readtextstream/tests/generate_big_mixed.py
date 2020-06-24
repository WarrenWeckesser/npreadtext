
import sys
import numpy as np


rng = np.random.default_rng()

# dt = np.dtype('i,i,i,i,i,i,i,i,i,i,U16,U16,U16,U16,f,f,f,f,f,f,f,f')
nrows = 2000000

filename = 'data/bigmixed.csv'
print("Generating {}".format(filename))

with open(filename, 'w') as f:
    for k in range(nrows):
        values1 = rng.integers(1, 1000, size=10).tolist()
        values2 = rng.choice(['abc', 'def', 'αβγ', 'apple', 'orange'], size=4).tolist()
        values3 = (rng.integers(0, 100, size=8)/8).tolist()
        values = values1 + values2 + values3
        s = ','.join(f'{v}' for v in values) + '\n'
        f.write(s)
        q, r = divmod(100*(k+1), nrows)
        if r == 0:
            print("\r{:3d}%".format(q), end='')
            sys.stdout.flush()
    print()
