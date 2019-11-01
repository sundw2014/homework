from scipy.special import comb
import numpy as np

N = 100

res = 0
for k in range(N):
    C1 = comb(N, k, exact=True)
    for m in range(int(np.ceil((N-k)/2))):
        res += C1 * comb(N-k, m, exact=True) * (0.1 ** k) * (0.5 ** m) * (0.4 ** (N-k-m))
print(res)
