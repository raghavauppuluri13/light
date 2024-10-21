import os
import platform
import subprocess

Environment()

# don't clutter src with .o files
VariantDir("build", "src", duplicate=False)

## Architecture name breakdown (arch)
## - larch64: linux tici aarch64
## - aarch64: linux pc aarch64
## - x86_64:  linux pc x64
## - Darwin:  mac x64 or arm64
real_arch = arch = subprocess.check_output(["uname", "-m"], encoding='utf8').rstrip()
is_linux = platform.system() != "Darwin"
if platform.system() == "Darwin":
    arch = "Darwin"
    brew_prefix = subprocess.check_output(['brew', '--prefix'], encoding='utf8').strip()
elif arch == "aarch64" and AGNOS:
    arch = "larch64"
assert arch in ["larch64", "aarch64", "x86_64", "Darwin"]
target_type = 'release'

# Dora
DORA_PATH = [f'external/dora/target/{target_type}']
DORA_LIBS = ['dora_node_api_c', 'm', 'rt', 'dl', 'pthread']

if arch == 'x86_64':
    cflags = ['-fdeclspec', '-fPIC', '-g', '-O3', '-mavx', '-mfma']
    cc = 'clang'
    libs = DORA_LIBS
    libpath = DORA_PATH
    cpppath = []
elif arch == 'Darwin':
    # not tested
    cflags = ['-fdeclspec', '-fPIC', '-g', '-O3']
    cc = 'clang'
    libs = DORA_LIBS
    libpath = DORA_PATH
    cpppath = []

if ARGUMENTS.get('debug', 0) == '1':
    cflags.append('-DDEBUG')

# Environment with C compiler
env = Environment(CC=cc, CCFLAGS=cflags, LIBPATH=libpath, LIBS=libs, CPPPATH=cpppath)

all_c_files = Glob('build/*.c')
c_files = [f for f in all_c_files if not f.name.endswith('node.c')]


# Nodes
def add_node(env, node_name):
    sources = c_files + [File(f'build/{node_name}.c')]
    objects = env.Object(sources)
    program = env.Program(target=f'build/{node_name}', source=sources)


nodes = [f for f in all_c_files if f.name.endswith('node.c')]

# auto-add + build nodes
for node in nodes:
    add_node(env, node.name[:-2])
