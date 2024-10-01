import numpy as np


def to_raw(msg, base_dtype):
    return np.frombuffer(msg, dtype=base_dtype)


def init(dtype: np.dtype, buffer=None):
    if buffer is not None:
        return np.frombuffer(buffer, dtype=dtype)
    return np.zeros(1, dtype=dtype)


def F32_ARRAY(dof: int):
    return np.dtype([
        ("data", np.dtype((np.float32, (dof)))),
    ])


# example msgs
def FUNCTION_DATA_ARRAY(size: int):
    return np.dtype([
        ("sin", np.dtype((np.float32, (size)))),
        ("cos", np.dtype((np.float32, (size)))),
        ("tan", np.dtype((np.float32, (size)))),
    ])


if __name__ == "__main__":
    msg = init(FUNCTION_DATA_ARRAY(6))
    msg["sin"] = np.sin(np.arange(6, dtype=np.float32))
    msg["cos"] = np.cos(np.arange(6, dtype=np.float32))
    msg["tan"] = np.tan(np.arange(6, dtype=np.float32))
    msg = to_raw(msg, np.float32)
    print(msg)
    print(msg.dtype)
    msg = init(FUNCTION_DATA_ARRAY(6), buffer=msg)
    for name in msg.dtype.names:
        arr = msg[name].flatten()
        for idx, val in enumerate(arr):
            print(f'{name}[{idx}] = {val}')
    print(msg)
    print(msg.dtype)
