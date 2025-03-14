from matplotlib import pyplot as plt
from matplotlib import animation

fig = plt.figure()

ax = plt.axes(xlim=(0, 2), ylim=(0, 100))

line, = plt.plot([], [])
count = 0

def init():
    line.set_data([], [])
    return line,

def animate(i):
    global count
    count = count + 1
    line.set_data([0, 2], [0,count])
    
    return line,

anim = animation.FuncAnimation(fig, animate, init_func=init,
                               frames=100, interval=20, blit=True)

plt.show()
