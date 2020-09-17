import matplotlib.pyplot as plt
import numpy as np

x = np.arange(0., 1., 0.001)
y1 = x ** (3/2)
y2 = x ** (2)

fig, ax = plt.subplots(1, 1, sharex=True)

ax.fill_between(x, y2, y1, facecolor='y', label='$<0$')
ax.plot(x, y1, 'r', label = r'$-x_1^3 + x_2^2 = 0$')
ax.plot(x, y2, 'b',label = r'$x_2 = x_1^2$')
ax.set_xlabel('$x_1$')
ax.set_ylabel('$x_2$')
ax.set_xlim(0,0.5)
ax.set_ylim(0,0.5)
plt.legend()
plt.show()

import matplotlib.pyplot as plt
import numpy as np

x = np.arange(0., 1., 0.001)
y1 = -np.log(1 - 0.5 * (x ** 2))
y2 = x ** (2)

fig, ax = plt.subplots(1, 1, sharex=True)

ax.fill_between(x, y1, y2, facecolor='y', label='$<1$')
ax.plot(x, y1, 'r', label = r'$\frac{1}{2}x_1^2 + e^{-x_2} = 1$')
ax.plot(x, y2, 'b',label = r'$x_2 = x_1^2$')
ax.set_xlabel('$x_1$')
ax.set_ylabel('$x_2$')
ax.set_xlim(0,0.5)
ax.set_ylim(0,0.5)
plt.legend()
plt.show()
