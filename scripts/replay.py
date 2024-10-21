from pathlib import Path
from light.utils.replay_buffer import ReplayBuffer
import light.utils.paths as PTH
import light.utils.constants as CFG
import mujoco
import mujoco.viewer
import rerun as rr
import click
import numpy as np


@click.command()
@click.option("--ep", type=int, help="episode start", default=0)
def main(ep):
    rr.init("read_check", spawn=True)
    replay_buffer = ReplayBuffer.create_from_path(
        (PTH.REC_PATH / CFG.RECORDING_EXP_NAME / "replay_buffer.zarr").as_posix(), mode="r")
    for i in range(ep, replay_buffer.n_episodes):
        ep = replay_buffer.get_episode(i)
        ep_lengths = replay_buffer.episode_lengths
        ep_start_idx = np.sum(ep_lengths[:i])
        timestamps = ep['timestamp']
        eplen = timestamps[-1] - timestamps[0]
        print(f"ep time length: {eplen}")
        print(f"sample rate: {len(timestamps) / eplen}")
        for j in range(ep_lengths[i]):
            rr.set_time_sequence('frame', ep_start_idx + j)
            for key, val in ep.items():
                if len(val) > 0:
                    if len(val.shape) > 1:
                        for k in range(val.shape[1]):
                            rr.log(f"{key.replace('-', '/')}/{k}", rr.Scalar(val[j, k]))


if __name__ == "__main__":
    main()
