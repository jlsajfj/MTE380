import fcntl
import os
import sys
import time
from threading import Thread
from tkinter import *

import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure

from helper import SB, Constants
from robot import Robot

if len(sys.argv) < 2:
    print("input serial port")
    sys.exit(1)


recording = False
data_t = []
data_speed1 = []
data_speed2 = []


def run():
    global data_t, data_speed1, data_speed2, recording

    data_t = []
    data_speed1 = []
    data_speed2 = []

    recording = True

    robot.send(f"set sp {kp.get()}")
    robot.send(f"set si {ki.get()}")
    robot.send(f"set sd {kd.get()}")
    robot.send(f"set sa {ka.get()}")
    robot.send("speed 6")

    time.sleep(1)

    robot.send("stop")

    recording = False

    ntime = np.array(data_t)
    nspeed1 = np.array(data_speed1)
    nspeed2 = np.array(data_speed2)

    time0 = ntime[(nspeed1 > 0).argmax(axis=0)]
    ntime -= time0

    ax.clear()
    ax.plot(ntime, nspeed1)
    ax.plot(ntime, nspeed2)
    ax.relim()

    canvas.draw()


# {{{ GUI
window = Tk()
window.title("Control Tuner")

frame = Frame(window)

run_btn = Button(frame, text="Run", command=run)
save_btn = Button(frame, text="Save", command=lambda: robot.send("save"))

stack = Frame(frame)

spinargs = {
    "master": stack,
    "from_": 0,
    "to": 5,
    "increment": 0.01,
}

kp = DoubleVar(value=0)
Label(stack, text="Kp").grid(row=0, column=0)
Spinbox(**spinargs, textvariable=kp).grid(row=0, column=1)

ki = DoubleVar(value=0)
Label(stack, text="Ki").grid(row=1, column=0)
Spinbox(**spinargs, textvariable=ki).grid(row=1, column=1)

kd = DoubleVar(value=0)
Label(stack, text="Kd").grid(row=2, column=0)
Spinbox(**spinargs, textvariable=kd).grid(row=2, column=1)

ka = DoubleVar(value=0)
Label(stack, text="Ka").grid(row=3, column=0)
Spinbox(**spinargs, textvariable=ka).grid(row=3, column=1)

stack.grid(row=0, column=0)
run_btn.grid(row=0, column=1, padx=10)
save_btn.grid(row=0, column=2, padx=10)

frame.grid(row=0, column=0, padx=10, pady=10)

fig = Figure()
ax = fig.add_subplot()

canvas = FigureCanvasTkAgg(fig, window)
canvas.get_tk_widget().grid(row=1, column=0, sticky="nsew", padx=10, pady=10)

window.grid_rowconfigure(1, weight=1)
window.grid_columnconfigure(0, weight=1)

# }}}


def read_thread():
    while robot.s is not None:
        start, data = robot.read()
        if recording and start == SB.STREAM.value:
            data_t.append(data["tis"])
            data_speed1.append(data["msl"])
            data_speed2.append(data["msr"])


robot = Robot(sys.argv[1], True)
robot.send("get")

while True:
    start, data = robot.read()

    if start == SB.CONFIG.value:
        kp.set(data["sp"])
        ki.set(data["si"])
        kd.set(data["sd"])
        ka.set(data["sa"])
        break

rt = Thread(target=read_thread)
rt.start()

window.mainloop()

robot.disconnect()
rt.join()

# vim: set foldmethod=marker:
