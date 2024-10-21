import toml
import click
from pathlib import Path
from datetime import datetime
import re
import light.utils.paths as PTH


def to_snake_case(name):
    pattern = re.compile(r'(?<!^)(?=[A-Z])')
    return pattern.sub('_', name).lower()


@click.command()
@click.argument('toml_path', type=click.Path(exists=True), required=True)
@click.argument('header_path',
                type=click.Path(),
                required=False,
                default=str(PTH.REPO_PATH / 'src' / 'constants.h'))
@click.argument('py_path',
                type=click.Path(),
                required=False,
                default=str(PTH.PKG_PATH / 'utils' / 'constants.py'))
def main(toml_path, header_path, py_path):
    # Load the TOML file
    with open(toml_path, 'r') as file:
        constants = toml.load(file)

    # Define path for the .c file and .py file
    c_file_path = Path(header_path).with_suffix('.c')
    py_file_path = Path(py_path).with_suffix('.py')
    c_file_path.unlink()
    py_file_path.unlink()
    py_file_path = py_file_path.as_posix()
    c_file_path = c_file_path.as_posix()
    # Start writing the C header file and C source file, and Python file
    with open(header_path, 'w') as header_file, open(c_file_path,
                                                     'w') as c_file, open(py_file_path,
                                                                          'w') as py_file:
        header_file.truncate(0)
        file_header_msg = f"Auto-generated constants from TOML configuration {toml_path} at {datetime.now()}\n\n"
        header_file.write("// " + file_header_msg)
        c_file.write("// " + file_header_msg)
        py_file.write("# " + file_header_msg)

        header_file.write("#ifndef CONSTANTS_H\n#define CONSTANTS_H\n\n")
        header_file.write("#include <stdint.h>\n\n")  # Include standard integer type
        c_file.write(
            f'#include "{Path(header_path).name}"\n\n')  # Include the header in the .c file

        # Write initial Python import statements

        # path to original config toml for back reference
        key = "TOML_PATH"
        header_file.write(f"#define CFG_{key} \"{toml_path}\"\n")
        py_file.write(f'{key.upper()} = "{toml_path}"\n')

        # Iterate through each section and generate definitions
        for section, values in constants.items():

            header_file.write("// -------------------------------\n")
            header_file.write(f"// Constants for {section}\n")
            header_file.write("// -------------------------------\n")
            assert isinstance(values, dict)
            for key, value in values.items():
                full_key_name = f"CFG_{section.upper()}_{key}".upper()
                py_key_name = f"{section.upper()}_{key}".upper()
                if isinstance(value, list):
                    if all(isinstance(x, str) for x in value):
                        element_type = "const char*"
                        value_str = ', '.join(f'"{x}"' for x in value)
                        py_value_str = ', '.join(f'"{x}"' for x in value)
                    elif all(isinstance(x, int) for x in value):
                        element_type = "int"
                        value_str = ', '.join(f"{x}" for x in value)
                        py_value_str = value_str
                    elif all(isinstance(x, float) for x in value):
                        element_type = "float"
                        value_str = ', '.join(f"{x}" for x in value)
                        py_value_str = value_str
                    header_file.write(f"extern {element_type} {full_key_name}[];\n")
                    c_file.write(f"{element_type} {full_key_name}[] = {{{value_str}}};\n")
                    py_file.write(f"{py_key_name} = [{py_value_str}]\n")
                elif isinstance(value, str):
                    header_file.write(f"#define {full_key_name} \"{value}\"\n")
                    py_file.write(f'{py_key_name} = "{value}"\n')
                elif isinstance(value, int):
                    header_file.write(f"#define {full_key_name} {value}\n")
                    py_file.write(f'{py_key_name} = {value}\n')
                elif isinstance(value, float):
                    header_file.write(f"#define {full_key_name} {value}\n")
                    py_file.write(f'{py_key_name} = {value}\n')

        header_file.write("#endif // CONSTANTS_H\n")


if __name__ == '__main__':
    main()
