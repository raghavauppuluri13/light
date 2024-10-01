""" 
## Inputs
- tick: Dora tick.
- image: HEIGHTxWIDTHxBGR array.
## Configuration

Use rerun to visualize data
"""

from typing import Callable
from dora import DoraStatus
from light.utils.messages import INIT_MSG, F32_ARRAY_MSG
import light.utils.constants as CFG
import os
import cv2
import numpy as np
import rerun as rr
from dora import Node


class Visualizer:

    def __init__(self):
        self.node = Node()
        rr.init("visualize")
        rr.spawn(memory_limit="100MB")

        motor_status = []

    def run(self):
        while True:
            for event in self.node:
                if event["type"] == "INPUT":
                    if event['id'] == 'image':
                        self.on_image_input(event)
                    elif event['id'] in ['timeseries']:
                        self.on_array_input(
                            event,
                            F32_ARRAY_MSG(CFG.CONTROL_NUM_JOINTS),
                        )
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
        frame = dora_input["value"].to_numpy().reshape((CFG.CAMERA_HEIGHT, CFG.CAMERA_WIDTH, 3))
        h0, w0, _ = frame.shape
        rr.log("camera/{}".format(dora_input['id']), rr.Image(frame.copy()))

    def on_array_input(
        self,
        dora_input: dict,
        dtype: np.dtype,
    ):
        stamp = dora_input['metadata']
        msg = INIT_MSG(dtype, buffer=dora_input["value"].to_numpy().copy())
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
