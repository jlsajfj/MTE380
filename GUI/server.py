import time
import serial
import struct


class robot:
    def __init__(self, dn: str = "COM4", con: bool = False):
        self.dn = dn
        self.s = None
        self.sw = None
        if con:
            self.connect()

    def connect(self) -> bool:
        if self.s is not None:
            return
        print("connecting to device")
        self.s = serial.Serial(self.dn)
        print("bluetooth device has been connected")

    def sync(self):
        self.s.reset_input_buffer()
        self.send("sync")
        s_cnt = 0
        while s_cnt < 5:
            b = self.s.read(1)

            if b == b"\x06":
                s_cnt += 1
            else:
                s_cnt = 0

    def read(self):# https://stackoverflow.com/a/7155595
        start = self.s.read(1)
        if start == b"\x0E":# stream
            data = self.s.read(25)
            print(struct.unpack("2b2biib6bbbI", data))
        if start == b"\x0F":# config
            data = self.s.read(53*8)
            print(struct.unpack("53d", data))

    def send(self):
        self.s.write((x + "\n").encode())

    def disconnect(self):
        print("disconnecting")
        self.s.close()
        self.s = None


def main():
    r = robot()
    r.connect()
    r.sync()
    r.read()
    input("press enter to continue")
    r.disconnect()


if __name__ == "__main__":
    main()
