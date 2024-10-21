import numpy as np


def msg_to_raw(msg):
    dtype = [str(v[0]) for v in msg.dtype.fields.values()][0]
    if "<f4" in dtype:
        base_dtype = np.dtype(np.float32)
    elif "<f8" in dtype:
        base_dtype = np.dtype(np.float64)
    elif "<i4" in dtype:
        base_dtype = np.dtype(np.int32)
    elif "<i8" in dtype:
        base_dtype = np.dtype(np.int64)
    else:
        base_dtype = np.dtype(dtype)
    return np.frombuffer(msg, dtype=base_dtype)


def msg_init(dtype: np.dtype, buffer=None):
    if buffer is not None:
        return np.frombuffer(buffer, dtype=dtype)
    return np.zeros(1, dtype=dtype)


def F32_ARRAY(dof: int):
    return np.dtype([
        ("data", np.dtype((np.float32, (dof)))),
    ])


if __name__ == "__main__":
    msg = init(F32_ARRAY(6))
    msg["data"] = np.sin(np.arange(6, dtype=np.float32))
    msg = msg_to_raw(msg)
    print(msg)
    print(msg.dtype)
    msg = msg_init(F32_ARRAY(6), buffer=msg)
    for name in msg.dtype.names:
        arr = msg[name].flatten()
        for idx, val in enumerate(arr):
            print(f'{name}[{idx}] = {val}')
    print(msg)
    print(msg.dtype)
