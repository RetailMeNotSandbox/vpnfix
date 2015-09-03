################################################################################
# General runtime variables.
# - UPPERCASE variables may be overriden at runtime.

## Directory variables.
top       := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
cwd       := $(shell pwd)
BUILD_DIR := $(cwd)/build

.PHONY: all build clean install

## Default target.
all: $(BUILD_DIR)/fixvpn

$(BUILD_DIR)/fixvpn: $(top)src/main.c | $(BUILD_DIR)
	$(CC) $^ -Wall -pedantic -I/usr/include -I$(top)include -o $@

$(BUILD_DIR):
	@mkdir -p $@

clean:
	@rm -rf $(BUILD_DIR)

install: all
	cp $(BUILD_DIR)/fixvpn /usr/local/bin/
