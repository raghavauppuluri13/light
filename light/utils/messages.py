# Auto-generated messages from TOML configuration messages/quickstart.toml at 2024-10-21 11:07:00.796546

import numpy as np
import light.utils.constants as CFG
def Pose():
	return np.dtype([
	("pos", np.dtype((np.float32,(3)))),
	("quat", np.dtype((np.float32,(4)))),
])
def Image():
	return np.dtype([
	("data", np.dtype((np.uint8,(CFG.CAMERA_WIDTH * CFG.CAMERA_HEIGHT * 3)))),
])
