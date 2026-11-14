#!/usr/bin/env python3
"""
Install SparkFun Pro nRF52840 Mini board support into Adafruit nRF52 Arduino Core.
Idempotent and non-destructive.
"""

import argparse
import os
import shutil
import subprocess
import sys

def get_default_arduino_dir():
    custom_dir = os.environ.get("ARDUINO_DIRECTORIES_DATA")
    if custom_dir and os.path.isdir(custom_dir):
        return custom_dir

    if sys.platform == "win32":
        local_app_data = os.environ.get("LOCALAPPDATA", "")
        if local_app_data:
            return os.path.join(local_app_data, "Arduino15")
    elif sys.platform == "darwin":
        return os.path.expanduser("~/Library/Arduino15")
    else:
        return os.path.expanduser("~/.arduino15")
    return None

def find_adafruit_nrf52_core(arduino_dir, target_version="1.7.0"):
    nrf52_base = os.path.join(arduino_dir, "packages", "adafruit", "hardware", "nrf52")
    if not os.path.isdir(nrf52_base):
        return None
    version_dir = os.path.join(nrf52_base, target_version)
    if os.path.isdir(version_dir):
        return version_dir
    # Fallback to any existing version
    versions = [os.path.join(nrf52_base, v) for v in os.listdir(nrf52_base) if os.path.isdir(os.path.join(nrf52_base, v))]
    if versions:
        return sorted(versions)[-1]
    return None

def install_board_support(repo_root, arduino_dir=None, target_version="1.7.0"):
    if not arduino_dir:
        arduino_dir = get_default_arduino_dir()

    if not arduino_dir or not os.path.isdir(arduino_dir):
        print(f"ERROR: Arduino data directory not found: {arduino_dir}", file=sys.stderr)
        return False

    core_dir = find_adafruit_nrf52_core(arduino_dir, target_version)
    if not core_dir:
        print(f"ERROR: Adafruit nRF52 core ({target_version}) not found in {arduino_dir}", file=sys.stderr)
        print("Please run 'arduino-cli core install adafruit:nrf52@1.7.0' first.", file=sys.stderr)
        return False

    print(f"Found Adafruit nRF52 core at: {core_dir}")

    # 1. Copy variant files
    hw_dir = os.path.join(repo_root, "hardware", "sparkfun_nrf52840_mini")
    variant_src_dir = hw_dir
    variant_dest_dir = os.path.join(core_dir, "variants", "sparkfun_nrf52840_mini")

    os.makedirs(variant_dest_dir, exist_ok=True)
    for fname in ["variant.h", "variant.cpp"]:
        src_file = os.path.join(variant_src_dir, fname)
        dst_file = os.path.join(variant_dest_dir, fname)
        if not os.path.exists(src_file):
            print(f"ERROR: Source variant file missing: {src_file}", file=sys.stderr)
            return False
        shutil.copy2(src_file, dst_file)
        print(f"Installed {fname} -> {dst_file}")

    # 2. Update boards.txt idempotently
    boards_file = os.path.join(core_dir, "boards.txt")
    if not os.path.isfile(boards_file):
        print(f"ERROR: boards.txt not found in {core_dir}", file=sys.stderr)
        return False

    fragment_file = os.path.join(hw_dir, "boards.txt.fragment")
    if not os.path.isfile(fragment_file):
        print(f"ERROR: boards.txt.fragment not found: {fragment_file}", file=sys.stderr)
        return False

    with open(boards_file, "r", encoding="utf-8", errors="replace") as f:
        boards_content = f.read()

    marker = "sparkfunnrf52840mini.name"
    if marker in boards_content:
        print("SparkFun Pro nRF52840 Mini entry already present in boards.txt (idempotent skip).")
    else:
        with open(fragment_file, "r", encoding="utf-8") as f:
            fragment_content = f.read()

        with open(boards_file, "a", encoding="utf-8") as f:
            f.write("\n\n" + fragment_content + "\n")
        print(f"Appended SparkFun board definition to {boards_file}")

    # 3. Verify with arduino-cli if in PATH
    try:
        res = subprocess.run(["arduino-cli", "board", "listall", "sparkfun"],
                             capture_output=True, text=True, check=False)
        if "sparkfunnrf52840mini" in res.stdout or "SparkFun Pro nRF52840 Mini" in res.stdout:
            print("Verified board support with arduino-cli: adafruit:nrf52:sparkfunnrf52840mini recognized.")
        else:
            print("Note: Board installed on disk. If arduino-cli board listall does not show it immediately, verify arduino-cli environment.")
    except FileNotFoundError:
        print("Note: arduino-cli not in system PATH for direct verification, but files installed successfully.")

    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Install SparkFun nRF52840 Mini support")
    parser.add_argument("--arduino-dir", help="Custom Arduino data directory")
    parser.add_argument("--core-version", default="1.7.0", help="Adafruit nRF52 core version")
    args = parser.parse_args()

    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    success = install_board_support(repo_root, args.arduino_dir, args.core_version)
    sys.exit(0 if success else 1)
