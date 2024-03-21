from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket
import time
from random import randint
import threading


class Handler(WebSocket):
    c_list = []
    callback = lambda x: print(x)
    def handleMessage(self):
        Handler.callback(self.data)

    def handleConnected(self):
        print(self.address, 'connected')
        Handler.c_list.append(self)

    def handleClose(self):
        print(self.address, 'closed')
        Handler.c_list.remove(self)

    @classmethod
    def broadcast(cls, msg: str):
        for c in cls.c_list:
            c.sendMessage(msg)

def main():
    server = SimpleWebSocketServer('', 8000, Handler)
    t1 = threading.Thread(target=server.serveforever)
    t1.start()
    input('press enter to continue')
    for i in range(20):
        Handler.broadcast(str({'cpu': {'name': i, 'value': randint(0,99)}}))
        time.sleep(0.1)
    t1.join()

if __name__ == '__main__':
    main()
