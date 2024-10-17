# light

Simple template repo with sane defaults for high performance, reliable hardware prototyping so you can iterate at the speed of light.

Uses dora as shared memory IPC communication backend. Use C for high performance stuff, and python for human interface tooling.

## Compatability
- Use pyenv for python version management and venv creation

### Tested
- python = 3.10.10
- linux = ubuntu 20.04

### Untested, but should work
- macos = (should work, but may need to adjust SConstruct to build on macos)

### Unsupported
- Windows

## Preliminaries

### Using C

All `src` files that end in `_node.c` have their own main file, and are compiled into `build`. For build simplicity, all files are kept in `src`.

Refer to examples in `src/examples`

### Using Python

All `scripts` files can be runnable. Write utility code and python infra modules in the `light` python package.

Refer to examples in `scripts/examples`

### Launching/Running code

Using dora's dataflow syntax, you can define all your node and their inputs/outputs in a single yaml file, see `launch/examples/quickstart.yml`

### Config Management

Each hardware configuration has a `config/(CONFIG_NAME).yml` file to fully describe all static parameters shared between python and c.
These are constants and are exported via `CONFIG=<CONFIG_NAME> make setup` or `CONFIG=<CONFIG_NAME> make` to be easily available via 
python: `import light.utils.constants as CFG` and c/c++: `#include "constants.h"`. Its just a simple python parser on top of toml.

### IPC

"Keep it simple. No abstractions. It's just float arrays and operations on float arrays." - Andrej Karpathy on llm.c

Found that it is sufficient to just send fixed size arrays of ints and floats between C and python and repackage them on either side.
For repackaging, see `light/utils/messages.py` for python and `src/messages.h(.c)` for C

Complex serialization isn't worth overoptimizing for simple applications when using shared memory IPC, but for more complex needs, use capnproto (TODO).

## Quick start
1. clone repo recursively: `git clone --recursive https://github.com/raghavauppuluri13/light`
1. install repo: `make install`
2. run `make`, default config is `config/quickstart.yml`, use `CONFIG=quickstart make` to compile with the quickstart config
3. run `dora up`
4. run `dora start launch/examples/quickstart.yml`

## Customize this repo

After cloning, replace all instances of `light` with `your_project` with `rename.sh light <PROJECT_NAME>`
