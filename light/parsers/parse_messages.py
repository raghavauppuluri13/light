import toml
import click
from pathlib import Path
from collections import defaultdict
from datetime import datetime
import re

import light.utils.constants as CFG
import light.utils.paths as PTH

TOML2NPY = {'f32': 'np.float32', 'u8': 'np.uint8', 'u32': "np.int32", 'u64': 'np.int64'}
TOML2C = {'f32': 'float', 'u8': 'uint8_t', 'u32': 'int', 'u64': 'long'}


def to_snake_case(name):
    pattern = re.compile(r'(?<!^)(?=[A-Z])')
    return pattern.sub('_', name).lower()


@click.command()
@click.argument('toml_path',
                type=click.Path(exists=True),
                required=False,
                default=str(Path(__file__).parent.parent / 'config' / 'messages.toml'))
@click.argument('header_path',
                type=click.Path(exists=True),
                required=False,
                default=str(PTH.REPO_PATH / 'src' / 'messages.h'))
@click.argument('py_path',
                type=click.Path(exists=True),
                required=False,
                default=str(PTH.PKG_PATH / 'utils' / 'messages.py'))
def main(toml_path, header_path, py_path):
    # Load the TOML file
    with open(toml_path, 'r') as file:
        messages = toml.load(file)

    # Define path for the .c file and .py file
    py_file_path = Path(py_path).with_suffix('.py')
    py_file_path.unlink()
    py_file_path = py_file_path.as_posix()
    # Start writing the C header file and C source file, and Python file
    with open(header_path, 'w') as header_file, open(py_file_path, 'w') as py_file:
        header_file.truncate(0)
        file_header_msg = f"Auto-generated messages from TOML configuration {toml_path} at {datetime.now()}\n\n"
        header_file.write("// " + file_header_msg)
        py_file.write("# " + file_header_msg)

        header_file.write("#ifndef MESSAGES_H\n#define MESSAGES_H\n\n")
        header_file.write("#include <stdint.h>\n\n")  # Include standard integer type
        header_file.write("#include \"common.h\"\n\n")  # include dora utility functions
        header_file.write("#include \"constants.h\"\n\n")
        py_file.write("import numpy as np\n")
        py_file.write("import light.utils.constants as CFG\n")

        # Iterate through each section and generate definitions
        for section, values in messages.items():
            header_file.write("// -------------------------------\n")
            header_file.write(f"// Type definition for {section}\n")
            header_file.write("// -------------------------------\n")
            assert isinstance(values, dict)
            header_file.write("typedef struct {\n")
            py_file.write(f"def {section}():\n")
            py_file.write("\treturn np.dtype([\n")
            tomltypes = defaultdict(int)
            total_size = 0
            for key, value in values.items():
                if isinstance(value, list):
                    assert len(value) == 2
                    tomltype, size = value
                    tomltypes[tomltype] += 1
                    ctype = TOML2C[tomltype]
                    pytype = TOML2NPY[tomltype]
                    header_file.write(f"\t{ctype} {key}[{size}];\n")
                    if not size.isnumeric():
                        # is constant
                        size = size.replace("CFG_", "CFG.")  # converts CFG_CONSTANT -> CFG.CONSTANT
                        total_size += eval(size)
                    else:
                        total_size += int(size)
                    py_file.write(f"\t(\"{key}\", np.dtype(({pytype},({size})))),\n")
                elif isinstance(value, str):
                    tomltype = value
                    tomltypes[tomltype] += 1
                    total_size += 1
                    ctype = TOML2C[tomltype]
                    header_file.write(f"\t{ctype} {key};\n")
                    pytype = TOML2NPY[tomltype]
                    py_file.write(f"\t(\"{key}\", {pytype}),\n")
                else:
                    raise ValueError("value must be str or list")
            header_file.write("} " + section + ";\n")
            py_file.write("])\n")

            # add message specific dora read/send macros
            if len(tomltypes.keys()) == 1:
                tomltype = list(tomltypes.keys())[0]
                header_file.write(
                    f"#define DORA_READ_{section}(event, arr) dora_read_{tomltype}_array(event, arr, {total_size})\n"
                )
                header_file.write(
                    f"#define DORA_SEND_{section}(dora_context, id, arr) dora_send_{tomltype}_array(dora_context, id, arr, {total_size})\n"
                )

            else:
                raise ValueError(
                    "Unsupported tomltype, only support one type per message. adding support for more complex messages soon with capnproto style encoding!",
                    tomltypes)

        header_file.write("#endif // MESSAGES_H\n")


if __name__ == '__main__':
    main()
