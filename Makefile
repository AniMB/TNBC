# Patient Monitor - Top-Level Makefile Orchestrator
# Enforces reproducible build environment, host unit tests, and firmware compilation.

PYTHON ?= python
CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -O2 -Iinclude

BUILD_DIR ?= build
BIN_DIR ?= $(BUILD_DIR)/bin
HOST_TEST_BIN ?= $(BUILD_DIR)/host_tests

# Source files for host unit test compilation
HOST_TEST_SRCS = \
	tests/host/test_runner.cpp \
	src/control/mode_coordinator.cpp \
	src/control/profile_map.cpp \
	src/control/profile_button.cpp \
	src/control/bp_worker.cpp \
	src/control/bp_button.cpp \
	src/tasks/ecg_task.cpp \
	src/tasks/spo2_task.cpp \
	src/tasks/status_task.cpp \
	src/tasks/power_task.cpp \
	src/tasks/sd_task.cpp

.PHONY: all setup doctor test firmware upload lint check clean help

all: check

help:
	@echo "Available Patient Monitor targets:"
	@echo "  make setup         - Check/install approved dependencies and custom board support"
	@echo "  make doctor        - Validate toolchain, pinned core, board FQBN, and hardware manifest"
	@echo "  make test          - Compile and run host unit tests (no physical board needed)"
	@echo "  make firmware      - Compile firmware binary for SparkFun Pro nRF52840 Mini"
	@echo "  make upload PORT=  - Upload compiled binary to specified serial port"
	@echo "  make lint          - Run static analysis and safety audit"
	@echo "  make check         - Execute doctor, test, lint, and firmware build"
	@echo "  make clean         - Remove repository build outputs"

setup:
	@echo "==> Setting up board support and toolchain..."
	$(PYTHON) scripts/install_board_support.py

doctor:
	@echo "==> Running environment and toolchain doctor..."
	$(PYTHON) scripts/verify_toolchain.py

test: $(HOST_TEST_BIN)
	@echo "==> Running host unit tests..."
	$(HOST_TEST_BIN)

$(HOST_TEST_BIN): $(HOST_TEST_SRCS)
	@mkdir -p $(BUILD_DIR) 2>/dev/null || $(PYTHON) -c "import os; os.makedirs('$(BUILD_DIR)', exist_ok=True)"
	$(CXX) $(CXXFLAGS) $(HOST_TEST_SRCS) -o $(HOST_TEST_BIN)

firmware:
	@echo "==> Building Patient Monitor firmware for SparkFun Pro nRF52840 Mini..."
	$(PYTHON) scripts/build_firmware.py

upload:
ifndef PORT
	$(error PORT is not set. Usage: make upload PORT=COM3 or PORT=/dev/ttyACM0)
endif
	@echo "==> Uploading firmware to $(PORT)..."
	$(PYTHON) scripts/build_firmware.py --upload $(PORT)

lint:
	@echo "==> Running code safety and static analysis audit..."
	$(PYTHON) scripts/lint_audit.py

check: doctor lint test firmware
	@echo "==> ALL CHECKS PASSED: Doctor, Lint, Unit Tests, and Firmware Build verified."

clean:
	@echo "==> Cleaning build artifacts..."
	$(PYTHON) -c "import shutil, os; shutil.rmtree('$(BUILD_DIR)', ignore_errors=True)"
