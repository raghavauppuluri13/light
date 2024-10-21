import os
import sys

from setuptools import setup, find_packages

setup(
    name="light",
    version="0.0.1",
    packages=find_packages(),
    description="Sane defaults for hardware prototyping with python and C",
    url="https://github.com/raghavauppuluri13/light",
    author="Raghava Uppuluri",
    install_requires=[
        "opencv-python",
        "numpy",
        "toml",
        "click",
        "rerun-sdk==0.18.2",
        "pyarrow",
        "pynput",
        "zerorpc",
        "maturin",
        "mujoco",
        "tomlkit",
        "zarr",
        "numcodecs",
    ],
)
