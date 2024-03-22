import re
import os


class Constants:
    SYNC_COUNT = 100
    SB_ACK = 0x06
    SB_NACK = 0x07
    SB_STREAM = 0x0E
    SB_CONFIG = 0x0F

    CONFIG_C = "Core/Src/config.c"
    STATE_H = "Core/Inc/state.h"

    CODE_MAP = {
        SB_ACK: "ACK",
        SB_NACK: "NACK",
        SB_STREAM: "STREAM",
        SB_CONFIG: "CONFIG",
    }
    HEADERS = [
        "msl",
        "msr",
        "mtl",
        "mtr",
        "mel",
        "mer",
        "pd0",
        "pd1",
        "pd2",
        "pd3",
        "pd4",
        "pd5",
        "bav",
        "mag",
    ]

    _STATE_NAMES = None
    _CONFIG_NAMES = None
    _STATE_MAP = None

    @classmethod
    @property
    def STATE_NAMES(cls):
        if cls._STATE_NAMES:
            return cls._STATE_NAMES

        state_re = re.compile(r"SM_STATE_(\w+)")
        path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), "..", cls.STATE_H
        )
        with open(path) as fd:
            cls._STATE_NAMES = state_re.findall(fd.read())

        return cls._STATE_NAMES

    @classmethod
    @property
    def CONFIG_NAMES(cls):
        if cls._CONFIG_NAMES:
            return cls._CONFIG_NAMES

        config_re = re.compile(r'\[CONFIG_ENTRY_[A-Z0-9_]+\]\s+=\s+{ "(\w+)"')
        path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)), "..", cls.CONFIG_C
        )
        with open(path) as fd:
            cls._CONFIG_NAMES = config_re.findall(fd.read())

        return cls._CONFIG_NAMES

    @classmethod
    @property
    def STATE_MAP(cls):
        if cls._STATE_MAP:
            return cls._STATE_MAP

        _STATE_MAP = dict(enumerate(cls.STATE_NAMES))
        return _STATE_MAP
