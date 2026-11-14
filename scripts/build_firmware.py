#!/usr/bin/env python3
"""
Firmware Build Orchestrator for Patient Monitor
Builds sketch for SparkFun Pro nRF52840 Mini using pinned Adafruit core and isolated staging.
"""

import argparse
import os
import shutil
import subprocess
import sys

FQBN = "adafruit:nrf52:sparkfunnrf52840mini"

def find_arduino_cli():
    cli = shutil.which("arduino-cli")
    if cli:
        return cli
    if sys.platform == "win32":
        default_win = r"C:\Program Files\Arduino CLI\arduino-cli.exe"
        if os.path.isfile(default_win):
            return default_win
    return None

def stage_sketch(repo_root, stage_dir):
    os.makedirs(stage_dir, exist_ok=True)
    # Copy PatientMonitor.ino
    src_ino = os.path.join(repo_root, "PatientMonitor.ino")
    dst_ino = os.path.join(stage_dir, "PatientMonitor.ino")
    shutil.copy2(src_ino, dst_ino)

    # Sync directories
    for d in ["src", "include", "config"]:
        src_path = os.path.join(repo_root, d)
        dst_path = os.path.join(stage_dir, d)
        if os.path.exists(dst_path):
            shutil.rmtree(dst_path)
        shutil.copytree(src_path, dst_path)

def build_firmware(upload_port=None):
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    arduino_cli = find_arduino_cli()
    if not arduino_cli:
        print("ERROR: arduino-cli not found. Run 'make setup' first.", file=sys.stderr)
        return False

    # Check board definition
    res = subprocess.run([arduino_cli, "board", "listall", "sparkfun"],
                         capture_output=True, text=True, check=False)
    if "sparkfunnrf52840mini" not in res.stdout:
        print("Board definition not found. Automatically running board installer...")
        install_script = os.path.join(repo_root, "scripts", "install_board_support.py")
        subprocess.run([sys.executable, install_script], check=True)

    build_dir = os.path.join(repo_root, "build")
    stage_dir = os.path.join(build_dir, "sketch", "PatientMonitor")
    output_dir = os.path.join(build_dir, "bin")

    os.makedirs(output_dir, exist_ok=True)
    stage_sketch(repo_root, stage_dir)

    print(f"Compiling Patient Monitor firmware for {FQBN}...")
    compile_cmd = [
        arduino_cli, "compile",
        "--fqbn", FQBN,
        "--output-dir", output_dir,
        stage_dir
    ]

    result = subprocess.run(compile_cmd, text=True)
    if result.returncode != 0:
        print("Firmware compilation failed.", file=sys.stderr)
        return False

    print(f"[+] Firmware compiled successfully into: {output_dir}")
    if os.path.exists(output_dir):
        for f in os.listdir(output_dir):
            fpath = os.path.join(output_dir, f)
            print(f"    - {f} ({os.path.getsize(fpath)} bytes)")

    if upload_port:
        print(f"Uploading firmware to port: {upload_port}...")
        upload_cmd = [
            arduino_cli, "upload",
            "-p", upload_port,
            "--fqbn", FQBN,
            "--input-dir", output_dir
        ]
        up_res = subprocess.run(upload_cmd, text=True)
        return up_res.returncode == 0

    return True

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Build Patient Monitor Firmware")
    parser.add_argument("--upload", help="Upload port (e.g. COM3 or /dev/ttyACM0)")
    args = parser.parse_args()

    success = build_firmware(upload_port=args.upload)
    sys.exit(0 if success else 1)
