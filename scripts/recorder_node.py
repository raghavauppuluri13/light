from typing import Callable
from dora import DoraStatus
from dora import Node
import os
import cv2
import numpy as np
import rerun as rr
import time
import zerorpc
import gevent

import light.utils.messages as MSG
from light.utils.timestamp_accumulator import TimestampObsAccumulator
from light.utils.replay_buffer import ReplayBuffer
import light.utils.constants as CFG
import light.utils.paths as PTH


class Recorder(object):

    def __init__(self, port=4344):
        self.node = Node()
        self.s = zerorpc.Server(self)
        self.s.bind(f"tcp://127.0.0.1:{port}")
        replay_buffer_path = PTH.REC_PATH / CFG.RECORDER_NAME
        replay_buffer_path.mkdir(parents=True, exist_ok=True)
        zarr_path = replay_buffer_path / "replay_buffer.zarr"
        self.replay_buffer = ReplayBuffer.create_from_path(zarr_path=zarr_path.as_posix(), mode="a")

        # recording buffers
        self.start_time = None
        self.accumulator = None
        self.data = {}
        self.stop = False

    def run(self):
        self.s.run()

    def spin(self):
        while not self.stop:
            for event in self.node:
                gevent.sleep(0.001)
                if event["type"] == "INPUT":
                    if event['id'] == 'tick':
                        self.update_episode()
                    elif event['id'] in [
                            'function_data',
                    ]:
                        self.on_array_input(
                            event,
                            MSG.FUNCTION_DATA(CFG.),
                        )
                elif event["type"] == "STOP":
                    print("Stopping!")
                    self.kill()
                else:
                    print("Unexpected event!")
                    self.kill()

    def start_episode(self):
        self.start_time = time.time()
        episode_id = self.replay_buffer.n_episodes

        # create accumulators
        self.accumulator = TimestampObsAccumulator(start_time=self.start_time, dt=0.01)
        return f"Episode {episode_id} started!"

    def update_episode(self):
        if self.accumulator is not None:
            self.accumulator.put(self.data, np.array([time.time()]))

    def end_episode(self):
        if self.accumulator is not None:
            data = self.accumulator.data
            timestamps = self.accumulator.timestamps
            n_steps = len(timestamps)
            if n_steps > 0:
                episode = dict()
                episode["timestamp"] = timestamps[:n_steps]
                for key, value in data.items():
                    episode[key] = value[:n_steps]
                self.replay_buffer.add_episode(episode, compressors="disk")
                episode_id = self.replay_buffer.n_episodes - 1
                self.accumulator = None
                return f"Episode {episode_id} saved!"
            return f"Episode episode is empty!"
        return "Episode not started!"

    def drop_episode(self):
        self.end_episode()
        self.replay_buffer.drop_episode()
        episode_id = self.replay_buffer.n_episodes
        return f"Episode {episode_id} dropped!"

    def on_array_input(
        self,
        dora_input: dict,
        dtype: np.dtype,
    ):
        msg = INIT_MSG(dtype, buffer=dora_input["value"].to_numpy().copy())
        for name in msg.dtype.names:
            arr = None
            if isinstance(msg[name], np.ndarray):
                arr = msg[name]
            elif isinstance(msg[name], (np.float32, np.float64, np.int32, np.int64)):
                arr = np.array([msg[name]])
            else:
                raise NotImplementedError
            self.data["{}-{}".format(dora_input['id'], name)] = arr

    def kill(self):
        self.end_episode()
        self.stop = True
        self.s.stop()
        return "killed!"


if __name__ == "__main__":
    rec = Recorder()
    gevent.spawn(rec.spin)
    rec.run()
