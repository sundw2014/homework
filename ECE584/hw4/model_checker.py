#importing libraries
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from scipy.spatial import ConvexHull, convex_hull_plot_2d
import numpy as np

# constants
a = 10.0
b = 8.0
epsilon = 0.05
delta_t = 0.05

# parameters
N = 2
T = 10
v0 = [1, 2]
u0 = [2, 1]
X = [[3, 5], [6,7]]
Y = [[3, 5], [6,7]]
unsafe_set_X = [2,3]
unsafe_set_Y = [2,3]

boundaries = [a, b]
colors = ['b','g','y']
sizes = [np.array([X[i][1]-X[i][0], Y[i][1]-Y[i][0]]) for i in range(N)]

def step_forward(x, v):
    # x: shape = (2,)
    x = x + delta_t * v
    # bounce/reflection
    for i in range(len(x)):
        if x[i]<0:
            x[i] = -x[i]
            v[i] = -v[i]
        if x[i]>boundaries[i]:
            x[i] = 2*boundaries[i]-x[i]
            v[i] = -v[i]
    return x, v

# initialization
vertices = [] # N*num_regions*(num_vertices*2)
velocities = [] # N*num_regions*(num_vertices*2)
for i in range(N):
    region = np.array([[X[i][0], Y[i][0]], [X[i][1], Y[i][0]], [X[i][0], Y[i][1]], [X[i][1], Y[i][1]]], dtype='float')
    vertices.append([region,])

    _velocities = np.array([[v0[i], u0[i]],] * 4, dtype='float')
    velocities.append([_velocities, ])

t = 0
fig = plt.figure()
#creating a subplot
ax1 = fig.add_subplot(1,1,1)

def calculate_true_reachable_set_from_vertices(vertices, size):
    rec = np.array([[vertices[:,0].min(), vertices[:,0].max()], [vertices[:,1].min(), vertices[:,1].max()]])
    _rec = np.zeros([2,2])
    _rec[0, 1] = boundaries[0] if rec[0, 0] + size[0] > boundaries[0] else rec[0, 1]
    _rec[0, 0] = 0 if rec[0, 1] - size[0] < 0 else rec[0, 0]
    _rec[1, 1] = boundaries[1] if rec[1, 0] + size[1] > boundaries[1] else rec[1, 1]
    _rec[1, 0] = 0 if rec[1, 1] - size[1] < 0 else rec[1, 0]
    return _rec

unsafe_point = None
unsafe_initialset_ID = None

result = 'safe'

def is_unsafe():
    global unsafe_point, unsafe_initialset_ID
    for i in range(N):
        for j in range(len(vertices[i])):
            rec = calculate_true_reachable_set_from_vertices(vertices[i][j], sizes[i])
            is_unsafe = (rec[0, 0]<unsafe_set_X[1] and rec[0, 1]>unsafe_set_X[0] and rec[1, 0]<unsafe_set_Y[1] and rec[1, 1]>unsafe_set_Y[0])
            unsafe_point = np.array([(max(rec[0,0], unsafe_set_X[0])+min(rec[0,1], unsafe_set_X[1]))/2, (max(rec[1,0], unsafe_set_Y[0])+min(rec[1,1], unsafe_set_Y[1]))/2])
            unsafe_initialset_ID = i
            if is_unsafe:
                return True
    return False

def plot_convex_hull(vertices, size, color):
    # hull = ConvexHull(vertices)
    # from IPython import embed; embed()
    # ax1.fill(vertices[hull.vertices,0], vertices[hull.vertices,1], color=color)
    rec = calculate_true_reachable_set_from_vertices(vertices, size)
    ax1.fill([rec[0,0], rec[0,0], rec[0,1], rec[0,1]], [rec[1,0], rec[1,1], rec[1,1], rec[1,0]], color=color)

    # print(vertices[hull.vertices,0], vertices[hull.vertices,1])

unsafe_flag = is_unsafe()

def simulate_single_point(x, T, v):
    t = 0
    while t<T:
        x, v = step_forward(x, v)
        t = t + delta_t
    return x, v

def is_point_in_rectangle(x, rec):
    return x[0]<rec[0,1] and x[0]>rec[0,0] and x[1]<rec[1,1] and x[1]>rec[1,0]

def backward_initial_check(x, T, v, initialset_ID):
    x, v = simulate_single_point(x, T, v)
    if is_point_in_rectangle(x, np.array([[X[initialset_ID][0], X[initialset_ID][1]], [Y[initialset_ID][0], Y[initialset_ID][1]]])) and np.abs(v.dot(np.array([v0[initialset_ID], u0[initialset_ID]]))/np.sum(v**2) + 1) < epsilon:
        return True, x, v
    else:
        return False, x, v

def animate(i):
    global t, vertices, velocities, unsafe_flag, result
    if t>T:
        print('result: is safe = %s'%(result))
        return
    ax1.clear()
    ax1.plot([0, 0, boundaries[0], boundaries[0], 0], [0, boundaries[1], boundaries[1], 0, 0], linewidth=3, color='k')
    ax1.fill([unsafe_set_X[0], unsafe_set_X[0], unsafe_set_X[1], unsafe_set_X[1]], [unsafe_set_Y[0], unsafe_set_Y[1], unsafe_set_Y[1], unsafe_set_Y[0]], color='r')
    if unsafe_flag:
        result = 'unsafe'
        print('unsafe')
        # from IPython import embed;embed()
        candidate_v = [np.array([v0[unsafe_initialset_ID], u0[unsafe_initialset_ID]]), np.array([v0[unsafe_initialset_ID], -u0[unsafe_initialset_ID]]), np.array([-v0[unsafe_initialset_ID], u0[unsafe_initialset_ID]]), np.array([-v0[unsafe_initialset_ID], -u0[unsafe_initialset_ID]])]
        # import pdb; pdb.set_trace()
        for j in range(4):
            _, x, v = backward_initial_check(unsafe_point, t, candidate_v[j], unsafe_initialset_ID)
            if _:
                print('unsafe initial set ID: %d, unsafe initial point: (%.3f, %.3f), velocity: (%.3f, %.3f)'%(unsafe_initialset_ID, x[0], x[1], -v[0], -v[1]))
                break

    for i in range(N):
        for j in range(len(vertices[i])):
            for k in range(len(vertices[i][j])):
                # from IPython import embed; embed()
                vertices[i][j][k,:], velocities[i][j][k,:] = step_forward(vertices[i][j][k,:], velocities[i][j][k,:])
            plot_convex_hull(vertices[i][j], sizes[i], colors[i])
    t = t + delta_t
    unsafe_flag = is_unsafe()
    # print(t)

anim = animation.FuncAnimation(fig, animate, interval=1, frames=int(T/delta_t)+1, repeat=False)

anim_running = True

def onClick(event):
    global anim_running
    if anim_running:
        anim.event_source.stop()
        anim_running = False
    else:
        anim.event_source.start()
        anim_running = True

fig.canvas.mpl_connect('button_press_event', onClick)

plt.show()
