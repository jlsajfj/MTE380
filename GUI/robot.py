import serial
import struct
import sys
import threading
import time
from enum import Enum
from helper import Constants


class Robot:
    def __init__(self, dn: str = "COM4", con: bool = False):
        self.config_names = Constants.CONFIG_NAMES
        self.dn = dn
        self.s = None
        self.sw = None
        self.send_lock = threading.Lock()
        self.debug = False
        self.synced = False
        if con:
            self.connect()

    def bug(self, val):
        self.debug = val

    def connect(self) -> bool:
        if self.s is not None:
            return
        print("connecting to device")
        self.s = serial.Serial(self.dn, 115200)
        self.synced = False
        print("bluetooth device has been connected")

        self.s.write(b'\n')
        self.read() # first sync

    def read(self):  # https://stackoverflow.com/a/7155595
        if self.debug:
            print("reading")

        State = Enum('DecoderState', ['SYNCING', 'START', 'STREAM', 'CONFIG'])
        state = State.START
        sync_cnt = 0

        if not self.synced:
            self.send('stream 1', True)
            state = State.SYNCING

        while self.s is not None:
            if state == State.SYNCING:
                if self.s.in_waiting < 1:
                    time.sleep(0.1)
                    continue

                ack = self.s.read(1)[0]
                if ack == Constants.SB_ACK:
                    sync_cnt += 1
                else:
                    sync_cnt = 0

                if sync_cnt >= Constants.SYNC_COUNT:
                    print("synced")
                    self.synced = True
                    state = State.START

            elif state == State.START:
                if self.s.in_waiting < 1:
                    time.sleep(0.1)
                    continue

                start = self.s.read(1)[0]

                if start == Constants.SB_ACK or start == Constants.SB_NACK:
                    if self.send_lock.locked():
                        self.send_lock.release()
                    return start, None

                elif start == Constants.SB_STREAM:
                    state = State.STREAM

                elif start == Constants.SB_CONFIG:
                    state = State.CONFIG

                else:
                    print(f"unknown start byte {start}")
                    self.send('stream 1', True)
                    state = State.SYNCING

            elif state == State.STREAM:
                if self.s.in_waiting < 25:
                    time.sleep(0.1)
                    continue

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

                return Constants.SB_STREAM, data_stream

            elif state == State.CONFIG:
                count = len(self.config_names)
                if self.s.in_waiting < count * 8:
                    time.sleep(0.1)
                    continue

                data = self.s.read(count * 8)
                config = dict(zip(self.config_names, struct.unpack(f"<{count}d", data)))

                return Constants.SB_CONFIG, config

        return Constants.SB_NACK, None

    def send(self, cmd, ignore_ack=False, timeout=1):
        print("sending", cmd)
        self.s.write(cmd.encode() + b"\n")
        if not ignore_ack:
            self.send_lock.acquire(timeout=timeout)

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
