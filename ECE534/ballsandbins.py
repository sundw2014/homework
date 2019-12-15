from scipy.special import comb
import numpy as np
from matplotlib import pyplot as plt

m = 100
n = 50

# res1 = 0
# res2 = 0
# for k in range(n+1):
#     if k < n:
#         res1 += k * (comb(n, k, exact=True) * ((1 - k/n) ** m) - comb(n, (k+1), exact=True) * ((1 - (k+1)/n) ** m))
#     res2 += k * comb(n, k, exact=True) * ((1 - 1/n) ** (m*k)) * ((1-((1 - 1/n) ** m)) ** (n-k))
#
# res3 = n * (1 - 1/n) ** m

iters = 1e4

def simulate(m, n):
    C = dict(zip(range(n), [0,]*n))
    for i in range(m):
        C[np.random.choice(n)] += 1
    return C

Xs = []

for i in range(int(iters)):
    C = simulate(m, n)
    X = 0
    for key in C:
        if C[key] == 0:
            X += 1
    Xs.append(X)

prob = lambda k: comb(n, k, exact=True) * ((1 - 1/n) ** (m*k)) * ((1-((1 - 1/n) ** m)) ** (n-k))

probs1 = [prob(k) for k in range(n+1)]
probs2 = [Xs.count(k)/iters for k in range(n+1)]

plt.plot(range(n+1), probs1, 'b')
plt.plot(range(n+1), probs2, 'r')
plt.show()
