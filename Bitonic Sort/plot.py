

import numpy as np
import matplotlib.pyplot as plt


a=np.loadtxt('serialRunningTimes.txt')
fig, ax = plt.subplots()
ax.scatter(a[:,0],a[:,1])
ax.plot(a[:,0],a[:,1])
ax.ticklabel_format(style='plain')

plt.ylabel('Running Time (in seconds)')
plt.xlabel('Number of Elements')
plt.title('Serial Running Time for Bitonic Sort')
plt.show()
