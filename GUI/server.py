import json
import sys
import threading

from SimpleWebSocketServer import SimpleWebSocketServer

from handler import Handler
from helper import SB, Filters, Positioner
from robot import Robot

if len(sys.argv) > 1:
    r = Robot(sys.argv[1])
else:
    r = Robot()
r.connect()
r.send("get", True)


def send_robot(message: str) -> None:
    if r.send(message):
        print("success")
    else:
        print("failed")


Handler.callback = send_robot


def send(code, data):
    packed_data = {
        "code": code.name,
        "data": data,
    }

    Handler.broadcast(json.dumps(packed_data))


server = SimpleWebSocketServer("", 8000, Handler)
t1 = threading.Thread(target=server.serveforever)
t1.start()

try:
    cnt = 0
    p_state = None
    f = Filters()
    p = Positioner()

    while True:
        code, data = r.read()
        # print(data)
        if code == SB.STREAM:
            data["bav"] *= 9 / 256
            f.process(data)

            if data["sta"] != p_state:
                send(code, f.get())
                p_state = data["sta"]
                if p_state == 2:
                    p = Positioner()

            p.update(f.get("mel"), f.get("mer"))
            cnt += 1
            if cnt >= 10:
                cnt = 0
                f_data = f.get()
                f_data["px"] = p.px
                f_data["py"] = p.py
                f_data["rx"] = p.rx
                f_data["ry"] = p.ry

                send(code, f_data)
        elif code == SB.CONFIG:
            print("config")
            Handler.cur_config = data
            send(code, data)
        else:
            send(code, data)
except KeyboardInterrupt:
    pass

server.close()
t1.join()

r.send("stream 0", True)
r.disconnect()
