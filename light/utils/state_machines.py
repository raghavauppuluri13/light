from enum import Enum, auto


class ProcessState(Enum):
    PAUSED = auto()
    RUNNING = auto()
    STOP = auto()
