import os
import re
import sys
import time
import serial
import struct

SYNC_COUNT = 50
SB_ACK    = 0x06
SB_STREAM = 0x0E
SB_CONFIG = 0x0F

CONFIG_C = "Core/Src/config.c"

class robot:
    def __init__(self, dn: str = "COM4", config_names = [], con: bool = False):
        self.config_names = config_names
        self.dn = dn
        self.s = None
        self.sw = None
        if con:
            self.connect()

    def connect(self) -> bool:
        if self.s is not None:
            return
        print("connecting to device")
        self.s = serial.Serial(self.dn, 115200)
        self.s.write(b'\n')
        print("bluetooth device has been connected")

    def sync(self):
        self.s.reset_input_buffer()
        self.send("sync")
        s_cnt = 0
        while s_cnt < SYNC_COUNT:
            b = self.s.read(1)[0]

            if b == SB_ACK:
                s_cnt += 1
            else:
                s_cnt = 0

    def read(self):# https://stackoverflow.com/a/7155595
        start = self.s.read(1)[0]
        if start == SB_ACK:
            # TODO check for ACK from send() here
            pass

        elif start == SB_STREAM:
            print("stream")
            data = self.s.read(25)
            print(struct.unpack("<2b2b2iB6BBBI", data))

        elif start == SB_CONFIG:
            print("config")
            count = len(self.config_names)
            data = self.s.read(count * 8)
            config = dict(zip(self.config_names, struct.unpack(f"<{count}d", data)))
            print(config)

        else:
            print(f"unknown start byte {start}")

    def send(self, cmd):
        self.s.write(cmd.encode() + b'\n')
        # TODO send() and read() should be in different threads
        # send() should block until read() gets an ACK in the right position
        while self.s.read(1)[0] != 0x06: pass

    def disconnect(self):
        print("disconnecting")
        self.s.close()
        self.s = None


def main():
    config_names = find_config_names()

    if len(sys.argv) > 1:
        r = robot(sys.argv[1], config_names=config_names)
    else:
        r = robot(config_names=config_names)

    r.connect()

    r.send("stream 1")
    r.sync()
    r.send("get")

    for i in range(20):
        r.read()

    r.disconnect()


def find_config_names():
    config_re = re.compile(r'\[CONFIG_ENTRY_[A-Z0-9_]+\]\s+=\s+{ "(\w+)"')
    path = os.path.join(os.path.dirname(os.path.realpath(__file__)), '..', CONFIG_C)
    with open(path) as fd:
        return config_re.findall(fd.read())


if __name__ == "__main__":
    main()
