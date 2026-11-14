#!/usr/bin/env bash
# Patient Monitor - Setup Script for Linux/macOS
# Pinned versions:
# - Adafruit nRF52 Core: 1.7.0
# - Target FQBN: adafruit:nrf52:sparkfunnrf52840mini

set -euo pipefail

echo "=========================================================="
echo "Setting up Patient Monitor Development Environment"
echo "=========================================================="

# 1. Check Python
if ! command -v python3 &>/dev/null; then
    echo "ERROR: python3 is required." >&2
    exit 1
fi
echo "[+] Python: $(python3 --version)"

# 2. Check Host C++ Compiler and Make
if ! command -v g++ &>/dev/null && ! command -v clang++ &>/dev/null; then
    echo "WARNING: Host C++ compiler (g++ or clang++) not found."
    echo "Install build-essential (Debian/Ubuntu) or Xcode command line tools (macOS)."
fi

if ! command -v make &>/dev/null; then
    echo "WARNING: make not found."
fi

# 3. Check or install Arduino CLI
if ! command -v arduino-cli &>/dev/null; then
    echo "[*] arduino-cli not found. Installing locally via official script..."
    curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR="${HOME}/.local/bin" sh
    export PATH="${HOME}/.local/bin:${PATH}"
fi
echo "[+] Arduino CLI: $(arduino-cli version)"

# 4. Configure Board Index and Install Adafruit nRF52 Core 1.7.0
ADAFRUIT_INDEX="https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
echo "[*] Updating core index..."
arduino-cli core update-index --additional-urls "${ADAFRUIT_INDEX}"

echo "[*] Installing pinned core adafruit:nrf52@1.7.0..."
arduino-cli core install adafruit:nrf52@1.7.0 --additional-urls "${ADAFRUIT_INDEX}"

# 5. Overlay SparkFun Board Support
echo "[*] Installing SparkFun Pro nRF52840 Mini board definition..."
python3 scripts/install_board_support.py

# 6. Verify Toolchain
echo "[*] Verifying toolchain..."
python3 scripts/verify_toolchain.py

echo "=========================================================="
echo "Setup complete!"
echo "=========================================================="
