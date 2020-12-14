import numpy as np
from scipy.linalg import expm
H = np.array([[0, 1 , 0 , 0], [0 , 0 , 0 , 0.5], [2 , 0 , 0 , 0], [0 , 2 , -1 , 0]])
C = np.random.randn(2,2)
M = C.T.dot(C)

t = -100.
res = expm(H*t).dot(np.concatenate([np.eye(2), -2*M], axis=0))
X = res[:2,:]
Y = res[2:,:]
P = -0.5*Y.dot(np.linalg.inv(X))
print(P)
