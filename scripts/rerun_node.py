from typing import Callable
from dora import DoraStatus, Node
from light.utils.message_utils import msg_init, msg_to_raw
import light.utils.messages as MSG
import light.utils.constants as CFG
import os
import cv2
import numpy as np
import rerun as rr


class Visualizer:

    def __init__(self):
        self.node = Node()
        rr.init("visualize")
        rr.spawn(memory_limit="100MB")

    def run(self):
        while True:
            for event in self.node:
                if event["type"] == "INPUT":
                    if event['id'] in ['image', 'fov']:
                        self.on_image_input(event)
                elif event["type"] == "STOP":
                    print("Stopping!")
                    return
                else:
                    print("Unexpected event!")
                    return

    def on_image_input(
        self,
        dora_input: dict,
    ):
        stamp = dora_input['metadata']
        msg = msg_init(MSG.Image(), buffer=dora_input["value"].to_numpy().copy())
        frame = msg["data"].reshape((CFG.CAMERA_HEIGHT, CFG.CAMERA_WIDTH, 1))
        print(frame.shape)
        h0, w0, _ = frame.shape
        rr.log("camera/{}".format(dora_input['id']), rr.Image(frame.copy()))

    def on_array_input(
        self,
        dora_input: dict,
        dtype: np.dtype,
    ):
        stamp = dora_input['metadata']
        msg = msg_init(dtype, buffer=dora_input["value"].to_numpy().copy())
        for name in msg.dtype.names:
            if isinstance(msg[name], np.ndarray):
                arr = msg[name].flatten()
                for idx, val in enumerate(arr):
                    rr.log("{}/{}/{}".format(dora_input['id'], name, idx), rr.Scalar(val))
            elif isinstance(msg[name], (np.float32, np.float64, np.int32, np.int64)):
                rr.log("{}/{}".format(
                    dora_input['id'],
                    name,
                ), rr.Scalar(msg[name]))
            else:
                raise ValueError(f"Unsupported type: {type(msg[name])}")


if __name__ == "__main__":
    vis = Visualizer()
    vis.run()
