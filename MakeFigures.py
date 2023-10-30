import matplotlib.pyplot as plt

import numpy as np

fileName = "PTestingTooMuch(200)"

dataY, dataX = np.genfromtxt(fileName+".txt",delimiter=",",skip_header=1,unpack=True)

plt.plot(dataX, dataY)
plt.xlabel("Time (sec)")
plt.ylabel("Temp (C)")
plt.savefig(fileName+".png")
plt.show()
