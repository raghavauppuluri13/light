""" 
# Teleop node
"""
from enum import Enum, auto
from typing import Callable
from dora import Node
import os
import numpy as np
from datetime import datetime
import pyarrow as pa
import itertools
import gevent
import zerorpc
import time
import mujoco
import mujoco.viewer
import light.utils.constants as CFG
from light.utils.traj_utils import generate_joint_space_min_jerk
from light.utils.messages import KIN_ARRAY_MSG, INIT_MSG, TO_RAW, F32_ARRAY_MSG
from light.utils.gello import Gello
from light.utils.inverse_kinematics import qpos_from_site_pose
from light.utils.config_mgmt import write_current_config
import light.utils.constants as CFG


class TeleopState(Enum):
    PAUSED = auto()
    CALIBRATE = auto()
    RUNNING = auto()
    STOP = auto()


class TeleopRPC(object):

    def __init__(self, port=4343):
        self.node = Node()
        self.s = zerorpc.Server(self)
        self.s.bind(f"tcp://127.0.0.1:{port}")
        self.teleop_state = TeleopState.PAUSED

        if CFG.GELLO_ENABLE:
            self.gello = Gello(baudrate=CFG.GELLO_BAUDRATE,
                               device_name=CFG.GELLO_DEVICE_NAME,
                               servo_ids=CFG.GELLO_SERVO_IDS)
            self.gello._disable_torque()
            gevent.spawn(self.gello_run)
        elif CFG.EEF_SERVO_ENABLE:
            gevent.spawn(self.eef_servo_run)

    def sanitize(self, target):
        if isinstance(target, str):
            return np.array([float(x) for x in target.split(",")])
        elif isinstance(target, float):
            return target
        else:
            raise TypeError("Inputs must be a float or a string ")

    '''----- Trajectory generation -----'''

    def cmd_range(self, target_type, traj_type, start, stop, total_time):
        target_type = target_type.lower()
        traj_type = traj_type.lower()
        if target_type not in ["joint", "motor"]:
            raise ValueError("target_type must be 'joint' or 'motor'")
        if traj_type not in ["pos", "eff"]:
            raise ValueError("traj_type must be 'pos' or 'eff'")
        start = self.sanitize(start)
        stop = self.sanitize(stop)
        total_time = self.sanitize(total_time)

        traj = generate_joint_space_min_jerk(start, stop, total_time, 0.001)

        for target in traj:
            target_msg = INIT_MSG(KIN_ARRAY_MSG(CFG.CONTROL_NUM_JOINTS))
            if traj_type == "pos":
                target_msg["pos"] = target['position']
                target_msg["vel"] = target['velocity']
            elif traj_type == "eff":
                target_msg["eff"] = target['position']
            self.node.send_output(
                f"{target_type}_targets",
                pa.array(TO_RAW(target_msg, np.float32)),
            )
            zerorpc.time.sleep(0.001)
        return "success!"

    def run(self):
        self.s.run()

    def cmd(self, target_type, traj_type, target):
        if target_type not in ["joint", "motor"]:
            raise ValueError("target_type must be 'joint' or 'motor'")
        if traj_type not in ["pos", "eff"]:
            raise ValueError("traj_type must be 'pos' or 'eff'")
        target = self.sanitize(target)
        target_msg = INIT_MSG(KIN_ARRAY_MSG(CFG.CONTROL_NUM_JOINTS))
        if traj_type == "pos":
            target_msg["pos"] = target['position']
            target_msg["vel"] = target['velocity']
        elif traj_type == "eff":
            target_msg["eff"] = target['position']
        self.node.send_output(
            f"{target_type}_targets",
            pa.array(TO_RAW(target_msg, np.float32)),
        )
        return "success!"

    def kp(self, data):
        data = self.sanitize(data)
        target_msg = INIT_MSG(F32_ARRAY_MSG(CFG.CONTROL_NUM_JOINTS))
        target_msg["data"] = data
        self.node.send_output(
            f"kp",
            pa.array(TO_RAW(target_msg, np.float32)),
        )
        return "success!"

    def kill(self):
        print("Cleaning up!")
        self.teleop_state = TeleopState.STOP
        self.s.stop()
        return "killed!"


if __name__ == "__main__":
    rpc = TeleopRPC()
    rpc.run()
