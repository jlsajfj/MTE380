from writer import ack, config, nack, stream

from robot import SB_ACK, SB_CONFIG, SB_NACK, SB_STREAM, Robot

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
