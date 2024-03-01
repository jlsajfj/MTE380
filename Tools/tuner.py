import sys, os, fcntl, time
from threading import Thread
from tkinter import *
from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

if(len(sys.argv) < 2):
    print("input serial port")
    sys.exit(1)

recording = False
data_t = []
data_speed = []

def command(cmd):
    fd.write(cmd.encode() + b'\n')
    time.sleep(0.1)
 
def run():
    global data_t, data_speed, recording

    data_t = []
    data_speed = []

    recording = True

    command(f'set sp {kp.get()}')
    command(f'set si {ki.get()}')
    command(f'set sd {kd.get()}')
    command('set sa 0.9')
    command('speed 6')

    time.sleep(0.2)

    command('stop')

    recording = False
 
    ax.clear()
    ax.plot(data_t, data_speed)
    ax.relim()
 
    canvas.draw()

# {{{ IO
buff = bytearray()
def read_thread():
    global buff
    while(not fd.closed):
        try:
            c = os.read(fd.fileno(), 1)
            buff += c
            if c == b'\n':
                if recording:
                    try:
                        [t, error, average, count1, count2, speed1, speed2, heading] = [float(f) for f in buff.decode().strip().split()]
                        data_t.append(t)
                        data_speed.append(speed1)
                    except ValueError:
                        pass
                elif b'=' in buff:
                    [param, value] = [f.strip() for f in buff.decode().split('=')]
                    value = float(value)

                    if param == 'sp':
                        kp.set(value)
                    if param == 'si':
                        ki.set(value)
                    if param == 'sd':
                        kd.set(value)

                buff = bytearray()
        except BlockingIOError:
            time.sleep(0.1)

fd = open(sys.argv[1], 'wb+', buffering=0)
fcntl.fcntl(fd.fileno(), fcntl.F_SETFL, os.O_NONBLOCK)

# }}}

# {{{ GUI
window = Tk()
window.title("Control Tuner")

frame = Frame(window)

button = Button(frame, text="Run", command=run)
 
stack = Frame(frame)

spinargs = {
    'master': stack,
    'from_': 0,
    'to': 5,
    'increment': 0.01,
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

stack.grid(row=0, column=0)
button.grid(row=0, column=1, padx=10)

frame.grid(row=0, column=0, padx=10, pady=10)

fig = Figure()
ax = fig.add_subplot()

canvas = FigureCanvasTkAgg(fig, window)
canvas.get_tk_widget().grid(row=1, column=0, sticky='nsew', padx=10, pady=10)

window.grid_rowconfigure(1, weight=1)
window.grid_columnconfigure(0, weight=1)
 
# }}}

command('echo off')

rt = Thread(target=read_thread)
rt.start()

command('debug off')
command('get')
command('debug on')
window.mainloop()
fd.close()
rt.join()

# vim: set foldmethod=marker:
