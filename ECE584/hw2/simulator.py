import numpy as np
import matplotlib.pyplot as plt
import argparse

parser = argparse.ArgumentParser(description="""""")

parser.add_argument('--n', type=int, help='number of variables')
parser.add_argument('--init_x', nargs='*', type=float, help='initial values of x')
parser.add_argument('--init_m', type=int, default=0, help='initial values of m [0..n-1] [default: 0]')
parser.add_argument('--max_steps', type=int, default=2e4, help='number of simulation steps [default: 2e4]')
parser.add_argument('--dt', type=float, default=1e-3, help='simulation time step [default: 1e-3]')
parser.add_argument('--h', type=float, default=3, help='h [default: 3]')
parser.add_argument('--a', nargs='*', type=float, help='a [default: [1.0] * n]')

args = parser.parse_args()
max_time_steps = int(args.max_steps)
dt = args.dt

N = args.n
a = args.a if args.a is not None else [1.0] * N
h = args.h

assert len(args.init_x) == N
assert 0 <= args.init_m and args.init_m < N

x = [[xi, ] for xi in args.init_x]
# x = [[1.0,], [2.0,], [3.0,], [4.0,], [5.0,]]
# x = [[1.0,] for i in range(N)]
m = args.init_m

_x  = np.array([xi[-1] for xi in x])
_x[m] = np.Inf
m_next = np.argmin(_x)

durations = [0.0] * N

# from IPython import embed; embed()
for step in range(max_time_steps-1):
    durations[m] += dt
    if x[m][-1] >= (1+h)*x[m_next][-1]:
        # import pdb; pdb.set_trace()
        m = m_next
        _x  = np.array([xi[-1] for xi in x])
        _x[m] = np.Inf
        m_next = np.argmin(_x)

    for i in range(N):
        if i == m:
            x[i].append(x[i][-1]*np.exp(dt*a[i]))
        else:
            x[i].append(x[i][-1])

print(('duration in each mode: ' + ', '.join(['{:.3f}']*N)).format(*durations))
print('T_max / T_min = %.3f'%(max(durations)/min(durations)))
time = np.arange(0, dt*max_time_steps, dt)

for i in range(N):
    plt.plot(time, np.array(x[i]), label=r'$x_%d$'%i)

plt.xlabel("Time (s)")
plt.ylabel("Value")
plt.xlim(left=0)
plt.title(r'$x_0$ = [' + ', '.join(['{:.1f}']*N).format(*args.init_x) + r']; $m_0$ = %d'%args.init_m + '; a = [' + ', '.join(['{:.1f}']*N).format(*a) + ']; h = %.3f'%args.h)
plt.legend(loc='upper right', frameon=True)
plt.show()
