from collections import deque
import math
import os
import re
from typing import Dict, List, Optional


class Constants:
    SYNC_COUNT: int = 10
    SB_SYNC: int = 0x05
    SB_ACK: int = 0x06
    SB_NACK: int = 0x07
    SB_STREAM: int = 0x0E
    SB_CONFIG: int = 0x0F

    CONFIG_C: str = "Core/Src/config.c"
    STATE_H: str = "Core/Inc/state.h"

    CODE_MAP: Dict[int, str] = {
        SB_ACK: "ACK",
        SB_NACK: "NACK",
        SB_STREAM: "STREAM",
        SB_CONFIG: "CONFIG",
    }

    FILTERS: Dict[str, int] = {
        "msl": 5,
        "msr": 5,
        "mtl": 5,
        "mtr": 5,
        "mel": 5,
        "mer": 5,
        "bav": 50,
    }

    HEADERS: List[str] = [
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
        "tis",
    ]

    _STATE_NAMES = None
    _CONFIG_NAMES = None
    _STATE_MAP = None

    @classmethod
    @property
    def STATE_NAMES(cls) -> List[str]:
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
    def CONFIG_NAMES(cls) -> List[str]:
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
    def STATE_MAP(cls) -> Dict[int, str]:
        if cls._STATE_MAP:
            return cls._STATE_MAP

        _STATE_MAP = dict(enumerate(cls.STATE_NAMES))
        return _STATE_MAP


class Filters:
    class ValueStore:
        def __init__(self, size: int):
            self.q: deque[float] = deque([0] * size)
            self.s: float = 0
            self.size: int = size

        def update(self, val: float):
            self.s -= self.q.popleft()
            self.s += val
            self.q.append(val)

            return self.s / self.size

    def __init__(self):
        self.vals: Dict[str, float] = dict(
            zip(Constants.HEADERS, [0] * len(Constants.HEADERS))
        )
        self.caches: Dict[str, self.ValueStore] = {}

        for f in Constants.FILTERS:
            self.caches[f] = self.ValueStore(Constants.FILTERS[f])

    def process(self, new_input: Dict[str, float]):
        self.vals = dict(new_input)

        for f in Constants.FILTERS:
            self.vals[f] = self.caches[f].update(new_input[f])

    def get(self, name: Optional[str] = None) -> Dict[str, float]:
        if name:
            return self.vals[name]

        return self.vals


class Positioner:
    # distance from wheels to center of bot in units of 100/ft
    m_dist = 15.625
    # conversion factor for map units per motor count
    unit_per_motor_count = 100 * math.pi * 28 / 12 / 25.4 / 30 / 20
    # half distance between wheels
    dw = 19
    # adjustment factor
    adj = 1.09
    # uneven adjustment
    a_l, a_r = 1.10, 1.00

    def __init__(
        self,
        x: float = 565.625,
        y: float = 550.0,
        el: float = 0,
        er: float = 0,
        t: float = math.pi,
    ):
        self.rx: float = x
        self.ry: float = y
        self.el: float = el
        self.er: float = er
        self.t: float = t

    @property
    def px(self) -> float:
        return self.rx + Positioner.m_dist * math.cos(self.t)

    @property
    def py(self) -> float:
        return self.ry + Positioner.m_dist * math.sin(self.t)

    def update(self, el: float, er: float) -> None:
        el *= Positioner.unit_per_motor_count * Positioner.adj * Positioner.a_l
        er *= Positioner.unit_per_motor_count * Positioner.adj * Positioner.a_r
        dl = el - self.el
        dr = er - self.er

        d = (dl + dr) / 2
        dt = (dr - dl) / (2 * Positioner.dw)

        self.rx += d * math.cos(self.t + dt / 2)
        self.ry += d * math.sin(self.t + dt / 2)
        self.t += dt

        self.el = el
        self.er = er
