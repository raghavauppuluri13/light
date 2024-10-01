#!/bin/sh
if [ -n "`$SHELL -c 'echo $ZSH_VERSION'`" ]; then
  ALIASES="$HOME/.zsh_aliases"
  RC="$HOME/.zshrc"
elif [ -n "`$SHELL -c 'echo $BASH_VERSION'`" ]; then
  ALIASES="$HOME/.bash_aliases"
  RC="$HOME/.bashrc"
else
    # User is using another shell
    echo "User is unsupported shell!"
    exit
fi
append_if_not_exists() {
    if ! grep -qF "$1" "$2"; then
        echo "appending '$1' to '$2'"
        echo "$1" >> "$2"
    fi
}

LIGHT_REPO_PATH=$(pwd) 

# download dora from source and build c ffi + cli

echo "building dora c api from source..."
cd external/dora
cargo build -p dora-node-api-c --release
cargo build -p dora-cli --release

# install python bindings
echo "building dora python bindings..."
pip install maturin
maturin develop -m apis/python/node/Cargo.toml

# add dora to PATH
echo "adding dora to PATH (if it doesn't exist)..."
append_if_not_exists "export PATH=\$PATH:$LIGHT_REPO_PATH/external/dora/target/release" $RC

# add mujoco lib to LD_LIBRARY_PATH
#echo "adding mujoco to LD_LIBRARY_PATH (if it doesn't exist)..."
#echo "Which mujoco version are you going to use? (currently supporting 3.0.1 and 3.0.0)"
#read mj_version
#append_if_not_exists "$LD_LIBRARY_PATH:$HOME/.mujoco/mujoco-$mj_version/lib"

cd $LIGHT_REPO_PATH
echo "Restart your terminal for install to take effect!"

# install linux can interfaces
#echo "initializing can hardware communication interface"
#sudo cp config/can/*.rules /etc/udev/rules.d/
#sudo udevadm control --reload-rules
#sudo cp config/can/*.network /etc/systemd/network
#sudo systemctl restart systemd-networkd
#echo "install done! unplug/plug the can adapters."
