import json
import threading
import time
from random import randint

from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket

from helper import Constants


class Handler(WebSocket):
    c_list = []
    callback = print
    cur_config = {}

    def handleMessage(self):
        Handler.callback(self.data)

    def handleConnected(self):
        print(self.address, "connected")
        packed_data = {
            "code": "STATE_MAP",
            "data": Constants.STATE_MAP,
        }
        # print(packed_data)
        self.sendMessage(json.dumps(packed_data))

        packed_data["code"] = "CONFIG"
        packed_data["data"] = Handler.cur_config
        # print(packed_data)
        self.sendMessage(json.dumps(packed_data))

        Handler.c_list.append(self)

    def handleClose(self):
        print(self.address, "closed")
        Handler.c_list.remove(self)

    @classmethod
    def broadcast(cls, msg: str):
        for c in cls.c_list:
            c.sendMessage(msg)


def main():
    server = SimpleWebSocketServer("", 8000, Handler)
    t1 = threading.Thread(target=server.serveforever)
    t1.start()
    input("press enter to continue")
    for i in range(20):
        Handler.broadcast(str({"cpu": {"name": i, "value": randint(0, 99)}}))
        time.sleep(0.1)
    t1.join()


if __name__ == "__main__":
    main()
