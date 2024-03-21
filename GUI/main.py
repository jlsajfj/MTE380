from handler import Handler
from robot import Robot, SB_ACK, SB_NACK, SB_STREAM, SB_CONFIG
import threading
from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket
import time
import json

c_map = {
    SB_ACK: 'ACK',
    SB_NACK: 'NACK',
    SB_STREAM: 'STREAM',
    SB_CONFIG: 'CONFIG',
}
server = SimpleWebSocketServer('', 8000, Handler)
t1 = threading.Thread(target=server.serveforever)
t1.start()
r = Robot('COM6')
r.connect()
r.send('stream 1', True)
time.sleep(0.1)
r.sync()
cnt = 0
headers = [ "msl", "msr", "mtl", "mtr", "mel", "mer", "sta", "pd0", "pd1", "pd2", "pd3", "pd4", "pd5", "bav", "mag", ]
vals = dict(zip(headers, [0.0] * len(headers)))
Handler.callback = r.send
def runGet():
    while True:
        r.send('get')
        time.sleep(1)
t2 = threading.Thread(target=runGet)
t2.start()
while True:
    code, data = r.read()
    # print(data)
    # print(vals)
    if code not in (SB_ACK, SB_NACK):
        if code == SB_STREAM:
            for h in headers:
                vals[h] += data[h]
            cnt += 1
            if cnt >= 10:
                for h in headers:
                    vals[h] /= 10
                vals['tis'] = data['tis']
                packed_data = {'code': c_map[code], 'data': vals}
                Handler.broadcast(json.dumps(packed_data))
                cnt = 0
                vals = dict(zip(headers, [0.0] * len(headers)))
        else:
            packed_data = {'code': c_map[code], 'data': data}
            Handler.broadcast(json.dumps(packed_data))
r.send("stream 0")
r.disconnect()
