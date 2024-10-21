import numpy as np
import logging
import time


class Hz:

    def __init__(self, print_hz=False, buffer_size=20, print_delay=2):
        self.buffer_size = buffer_size
        self.print_delay = print_delay
        self.last_t = None
        self.hz = None
        self.buffer = np.zeros(self.buffer_size)
        self.b_i = 0

        self.last_print_t = None
        self.print_hz = print_hz

    def clock(self):
        if self.last_t is None:
            self.last_t = time.perf_counter_ns()
            return
        dt = time.perf_counter_ns() - self.last_t
        self.buffer[self.b_i] = dt / 1e6
        self.b_i = (self.b_i + 1) % self.buffer_size
        self.last_t = time.perf_counter_ns()

        if self.last_print_t is None:
            self.last_print_t = time.time()
        elif self.print_hz and time.time() - self.last_print_t > self.print_delay:
            logging.info(f"latency: {self.get_hz()}ms")
            self.last_print_t = time.time()

    def get_hz(self):
        return self.buffer.mean()
