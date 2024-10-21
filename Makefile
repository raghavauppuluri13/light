# Example usage: CONFIG=quickstart make

.PHONY: all debug setup clean build

CONFIG ?= quickstart
SCONS_BUILD ?= export CONFIG=$(CONFIG); export PATH=$(shell pwd)/external/dora/target/release:$$PATH; scons

all: setup
	$(SCONS_BUILD)

build: 
	python3 -m light.parsers.parse_config config/$(CONFIG).toml
	python3 -m light.parsers.parse_messages messages/$(CONFIG).toml
	$(SCONS_BUILD)

debug: setup
	$(SCONS_BUILD) debug=1

clean:
	$(SCONS_BUILD) -c

setup:
	python3 -m light.parsers.parse_config config/$(CONFIG).toml
	python3 -m light.parsers.parse_messages messages/$(CONFIG).toml

install:
	./install.sh
	pip3 install -e .
