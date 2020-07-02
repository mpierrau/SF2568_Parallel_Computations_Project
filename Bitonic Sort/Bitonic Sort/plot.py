

import numpy as np
import matplotlib.pyplot as plt


a=np.loadtxt('serialRunningTimes.txt')
# fig, ax = plt.subplots()
# ax.scatter(a[:,0],a[:,1])
# ax.plot(a[:,0],a[:,1])
# ax.ticklabel_format(style='plain')

b=np.loadtxt('serialRunningTimes_sorted.txt')
# fig, ax = plt.subplots()
# ax.scatter(b[:,0],b[:,1])
# ax.plot(b[:,0],b[:,1])
# ax.ticklabel_format(style='plain')

c=np.loadtxt('serialRunningTimes_bad.txt')
# fig, ax = plt.subplots()
# ax.scatter(c[:,0],c[:,1])
# ax.plot(c[:,0],c[:,1])
# ax.ticklabel_format(style='plain')

# plt.ylabel('Running Time (in seconds)')
# plt.xlabel('Number of Elements')
# plt.title('Serial Running Time for Bitonic Sort')
# plt.show()


plt.plot(b[:,0],b[:,1],label='Sorted Sequence')
plt.plot(a[:,0],a[:,1],label='Random Sequence')
plt.plot(c[:,0],c[:,1],label='Reverse Sequence')
plt.legend()
plt.ylabel('Running Time (in seconds)')
plt.xlabel('Number of Elements')
plt.title('Performance on sorted array vs Random Array')
plt.show()