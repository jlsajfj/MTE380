from robot import Robot, SB_ACK, SB_NACK, SB_STREAM, SB_CONFIG
from writer import ack, nack, stream, config

func_map = {
        SB_ACK: ack,
        SB_NACK: nack,
        SB_STREAM: stream,
        SB_CONFIG: config,
}

def main():
    r = Robot("COM5")
    r.connect()
    r.send("stream 1")
    r.sync()
    r.send("get")
    while True:
        code, data = r.read()
        func_map[code].apply_async(args=[data])

if __name__ == "__main__":
    main()
