# Example usage: CONFIG=quickstart make

.PHONY: all debug setup clean build

CONFIG ?= quickstart
CFG_PATH = $(shell pwd)/config/$(CONFIG).toml
SCONS_BUILD ?= export CONFIG=$(CONFIG); export PATH=$(shell pwd)/external/dora/target/release:$$PATH; scons

all: setup
	$(SCONS_BUILD)

build: 
	python3 config/parse_toml.py config/$(CONFIG).toml
	$(SCONS_BUILD)

debug: setup
	$(SCONS_BUILD) debug=1

clean:
	$(SCONS_BUILD) -c

setup:
	pip3 install -e .
	python3 config/parse_toml.py config/$(CONFIG).toml

install:
	./install.sh
