from handler import Handler
from robot import Robot
import threading
from SimpleWebSocketServer import SimpleWebSocketServer
import sys
import time
import json
from helper import Constants

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
r.sync()

cnt = 0
vals = dict(zip(Constants.HEADERS, [0.0] * len(Constants.HEADERS)))
Handler.callback = r.send


def runGet():
    while True:
        r.send("get")
        time.sleep(1)


t2 = threading.Thread(target=runGet)
t2.start()

p_state = None


def send(code, data):
    packed_data = {
        "code": Constants.CODE_MAP[code],
        "data": data,
    }

    Handler.broadcast(json.dumps(packed_data))


while True:
    code, data = r.read()
    # print(data)
    # print(vals)
    if code not in (Constants.SB_ACK, Constants.SB_NACK):
        if code == Constants.SB_STREAM:
            if data["sta"] != p_state:
                send(code, data)
                p_state = data["sta"]

            for h in Constants.HEADERS:
                vals[h] += data[h]

            cnt += 1
            if cnt >= 10:
                for h in Constants.HEADERS:
                    vals[h] /= 10

                vals["tis"] = data["tis"]
                vals["sta"] = data["sta"]
                vals["bav"] *= 9.0 / 256
                send(code, vals)
                cnt = 0
                vals = dict(zip(Constants.HEADERS, [0.0] * len(Constants.HEADERS)))
        else:
            send(code, data)

r.send("stream 0")
r.disconnect()
