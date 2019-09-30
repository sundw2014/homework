import numpy as np
N = 5
x = [1.0, 2.0, 3.0, 4.0, 5.0]
# x = [1.0] * N
m = 0
h = 10
max_switching_times = 10
execution = ''
_execution = ''
t = 0
for i in range(max_switching_times):
    execution += '%d [%f, '%(m, t)
    _x  = np.array(x)
    _x[m] = np.Inf
    j = np.argmin(_x)
    delta_t = np.log((1+h)*_x[j]/x[m])
    _execution += '(%d, %.3f) $rightarrow$'%(m, delta_t)
    x[m] = (1+h)*_x[j]
    t += delta_t
    m = j
    execution += '%f] '%(t)
print(_execution)
print(execution)
