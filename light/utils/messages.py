# Auto-generated messages from TOML configuration messages/quickstart.toml at 2024-10-20 22:58:05.127348

import numpy as np
import light.utils.constants as CFG


def Quaternion():
    return np.dtype([
        ("x", np.float32),
        ("y", np.float32),
        ("z", np.float32),
        ("w", np.float32),
    ])


def Vector3():
    return np.dtype([
        ("x", np.float32),
        ("y", np.float32),
        ("z", np.float32),
    ])


def Image():
    return np.dtype([
        ("pix", np.dtype((np.uint8, (CFG.CAMERA_WIDTH * CFG.CAMERA_HEIGHT)))),
    ])
