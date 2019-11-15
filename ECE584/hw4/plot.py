import matplotlib
import matplotlib.pyplot as plt
import numpy as np

t = [0, 1, np.nan, 1, 2, np.nan, 2, 3, np.nan, 3, 4, np.nan, 4, 5, np.nan, 5]
x = [0, 2, np.nan, 4, 1, np.nan, 1, -2, np.nan, -2, 0, np.nan, 0, 2, np.nan, 4]
y = [0, -3, np.nan, -3, -6, np.nan, -4, -2, np.nan, -2, 0, np.nan, -1, -4, np.nan, -4]
x_l = [0, 2, np.nan, 4, 1, np.nan, 1, -2, np.nan, -2, 0, np.nan, 0, 2, np.nan, 4]
x_u = [0, 2, np.nan, 4, 1, np.nan, 1, -2, np.nan, -1, 1, np.nan, 1, 3, np.nan, 4]
y_l = [0, -3, np.nan, -3, -6, np.nan, -4, -2, np.nan, -2, 0, np.nan, -1, -4, np.nan, -4]
y_u = [0, -3, np.nan, -3, -6, np.nan, -4, -2, np.nan, -2, 0, np.nan, 1, -2, np.nan, -3]

fig, ax = plt.subplots()
ax.set(xlabel='time', ylabel='value')
ax.grid()
plt.plot(t, x, 'r-', label=r'$x$')
plt.plot(t, y, 'g-',  label=r'$y$')
plt.plot(t, x_l, 'r:',  label=r'$x_l$')
plt.plot(t, x_u, 'r--',  label=r'$x_u$')
plt.plot(t, y_l, 'g:',  label=r'$y_l$')
plt.plot(t, y_u, 'g--',  label=r'$y_u$')
for data in [x, y, x_l, x_u, y_l, y_u]:
    for i in range(1, 6):
        index = (i*3-1)
        if data[index-1] != data[index+1]:
            plt.plot(t[index-1], data[index-1], 'ko', markerfacecolor='white', markeredgewidth=1.0)
            plt.plot(t[index+1], data[index+1], 'ko')
plt.legend()
plt.show()
