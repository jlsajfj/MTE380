import os
import re
import serial
import struct
import sys
import threading
import time

SYNC_COUNT = 50
SB_ACK = 0x06
SB_NACK = 0x07
SB_STREAM = 0x0E
SB_CONFIG = 0x0F

CONFIG_C = "Core/Src/config.c"


class Robot:
    def __init__(self, dn: str = "COM4", con: bool = False):
        self.config_names = find_config_names()
        self.dn = dn
        self.s = None
        self.sw = None
        self.send_lock = threading.Lock()
        self.debug = False
        if con:
            self.connect()


    def bug(val):
        self.debug = val

    def connect(self) -> bool:
        if self.s is not None:
            return
        print("connecting to device")
        self.s = serial.Serial(self.dn, 115200)
        self.s.write(b"\n")
        print("bluetooth device has been connected")

    def sync(self):
        print("syncing")
        self.s.reset_input_buffer()
        self.send("sync", True)
        s_cnt = 0
        if self.debug: print("starting loop")
        while s_cnt < SYNC_COUNT:
            b = self.s.read(1)[0]

            if b == SB_ACK:
                s_cnt += 1
            else:
                s_cnt = 0
            # print(time.time(), "s_cnt:", s_cnt)
        print("synced")

    def read(self):  # https://stackoverflow.com/a/7155595
        if self.debug: print('reading')
        start = self.s.read(1)[0]
        if start == SB_ACK or start == SB_NACK:
            if self.send_lock.locked():
                self.send_lock.release()
            return start, None

        elif start == SB_STREAM:
            if self.debug: print("stream")
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

        elif start == SB_CONFIG:
            if self.debug: print("config")
            count = len(self.config_names)
            data = self.s.read(count * 8)
            config = dict(zip(self.config_names, struct.unpack(f"<{count}d", data)))
            return start, config

        else:
            print(f"unknown start byte {start}")
            self.sync()

    def send(self, cmd, ignore_ack=False):
        print("sending",cmd)
        if not ignore_ack:
            self.send_lock.acquire()
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
    r.sync()
    r.send("get")

    for i in range(20):
        r.read()

    r.send("stream 0", True)
    r.disconnect()


def find_config_names():
    config_re = re.compile(r'\[CONFIG_ENTRY_[A-Z0-9_]+\]\s+=\s+{ "(\w+)"')
    path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", CONFIG_C)
    with open(path) as fd:
        return config_re.findall(fd.read())


if __name__ == "__main__":
    main()
