from handler import Handler
from robot import Robot
import threading
from SimpleWebSocketServer import SimpleWebSocketServer
import sys
import json
from helper import Constants, Filters, Positioner

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
        "code": Constants.CODE_MAP[code],
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
        if code == Constants.SB_STREAM:
            data["bav"] *= 9 / 256
            f.process(data)

            if data["sta"] != p_state:
                send(code, f.get())
                p_state = data["sta"]

            cnt += 1
            if cnt >= 10:
                p.update(f.get("mel"), f.get("mer"))
                cnt = 0
                f_data = f.get()
                f_data["px"] = p.tx
                f_data["py"] = p.ty

                send(code, f_data)
        elif code == Constants.SB_CONFIG:
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
