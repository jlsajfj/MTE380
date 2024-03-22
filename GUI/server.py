from handler import Handler
from robot import Robot
import threading
from SimpleWebSocketServer import SimpleWebSocketServer
import sys
import time
import json
from helper import Constants, Filters

server = SimpleWebSocketServer("", 8000, Handler)
t1 = threading.Thread(target=server.serveforever)
t1.start()

if len(sys.argv) > 1:
    r = Robot(sys.argv[1])
else:
    r = Robot()

r.connect()
r.send("stream 1", True)
time.sleep(0.1)
r.send("sync")

cnt = 0
Handler.callback = r.send


def runGet():
    while True:
        r.send("get")
        time.sleep(1)
        print(time.time())


t2 = threading.Thread(target=runGet)
t2.start()

p_state = None


def send(code, data):
    packed_data = {
        "code": Constants.CODE_MAP[code],
        "data": data,
    }

    Handler.broadcast(json.dumps(packed_data))


f = Filters()
while True:
    code, data = r.read()
    # print(data)
    if code not in (Constants.SB_ACK, Constants.SB_NACK):
        if code == Constants.SB_STREAM:
            data["bav"] *= 9 / 256
            f.process(data)

            if data["sta"] != p_state:
                send(code, f.get())
                p_state = data["sta"]

            cnt += 1
            if cnt >= 10:
                send(code, f.get())
                cnt = 0
        else:
            send(code, data)

r.send("stream 0")
r.disconnect()
