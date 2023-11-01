import matplotlib.pyplot as plt

import numpy as np

fileName = "PTestingTooMuch(200)"

Temp, PWM, SetTemp, Time = np.genfromtxt(fileName+".txt",delimiter=",",skip_header=1,unpack=True)

figure = plt.figure()
subfigs = figure.subplots(2,1,sharex=True)

subfigs[0].plot(Time,Temp,label="Measurement")

subfigs[0].set_ylabel("Temp (C)")

subfigs[0].plot(Time,SetTemp,label="Set Temp")

subfigs[0].set_title("Output")

subfigs[0].legend()

subfigs[1].plot(Time,PWM)

subfigs[1].set_ylabel("PWM")

subfigs[1].set_title("Input")

figure.supxlabel("Time (sec)")

figure.savefig(fileName+".png")

plt.show()
