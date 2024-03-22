import serial
import struct
import sys
import threading
from helper import Constants


class Robot:
    def __init__(self, dn: str = "COM4", con: bool = False):
        self.config_names = Constants.CONFIG_NAMES
        self.dn = dn
        self.s = None
        self.sw = None
        self.send_lock = threading.Lock()
        self.debug = False
        self.sync_cnt = 0
        if con:
            self.connect()

    def bug(self, val):
        self.debug = val

    def connect(self) -> bool:
        if self.s is not None:
            return
        print("connecting to device")
        self.s = serial.Serial(self.dn, 115200)
        self.s.write(b"\n")
        print("bluetooth device has been connected")

    def read(self):  # https://stackoverflow.com/a/7155595
        if self.debug:
            print("reading")

        start = self.s.read(1)[0]
        # print(self.sync_cnt)

        if start == Constants.SB_ACK or start == Constants.SB_NACK:
            self.sync_cnt += 1
            if self.sync_cnt >= Constants.SYNC_COUNT:
                self.s.reset_input_buffer()
                self.sync_cnt = 0

            if self.send_lock.locked():
                self.send_lock.release()

            return start, None
        elif start == Constants.SB_STREAM:
            self.sync_cnt = 0

            if self.debug:
                print("stream")

            data = self.s.read(25)
            data_stream = dict(
                zip(
                    [
                        "msl",
                        "msr",
                        "mtl",
                        "mtr",
                        "mel",
                        "mer",
                        "sta",
                        "pd0",
                        "pd1",
                        "pd2",
                        "pd3",
                        "pd4",
                        "pd5",
                        "bav",
                        "mag",
                        "tis",
                    ],
                    struct.unpack("<2b2b2iB6BBBI", data),
                )
            )

            return start, data_stream

        elif start == Constants.SB_CONFIG:
            self.sync_cnt = 0

            if self.debug:
                print("config")

            count = len(self.config_names)
            data = self.s.read(count * 8)
            config = dict(zip(self.config_names, struct.unpack(f"<{count}d", data)))

            return start, config

        else:
            self.sync_cnt = 0

            print(f"unknown start byte {start}")
            self.send("stream 1", True)
            return Constants.SB_NACK, None

    def send(self, cmd, ignore_ack=False):
        print("sending", cmd)
        if not ignore_ack:
            self.send_lock.acquire(timeout=5)
        self.s.write(cmd.encode() + b"\n")

    def disconnect(self):
        print("disconnecting")
        self.s.close()
        self.s = None


def main():
    if len(sys.argv) > 1:
        r = Robot(sys.argv[1])
    else:
        r = Robot()

    r.connect()

    r.send("stream 1")
    r.send("sync")
    r.send("get")

    for i in range(20):
        r.read()

    r.send("stream 0", True)
    r.disconnect()


if __name__ == "__main__":
    main()
