#!/usr/bin/env python3
"""
Verify developer toolchain, installed Arduino core, board support, and hardware manifest.
Used by 'make doctor' and CI checks.
"""

import os
import re
import shutil
import subprocess
import sys

def check_command(cmd, name, version_arg="--version", version_pattern=None):
    executable = shutil.which(cmd)
    if not executable:
        print(f"[-] {name} ({cmd}): NOT FOUND")
        return False, None
    try:
        res = subprocess.run([cmd, version_arg], capture_output=True, text=True, check=False)
        output = (res.stdout + " " + res.stderr).strip()
        version = output.splitlines()[0] if output else "found"
        if version_pattern:
            m = re.search(version_pattern, output)
            if m:
                version = m.group(0)
        print(f"[+] {name} ({cmd}): {version} [{executable}]")
        return True, version
    except Exception as e:
        print(f"[!] {name} ({cmd}): found at {executable} but error checking version: {e}")
        return True, "unknown"

def check_arduino_board(fqbn="adafruit:nrf52:sparkfunnrf52840mini"):
    executable = shutil.which("arduino-cli")
    if not executable:
        print(f"[-] Board check skipped: arduino-cli not found in PATH")
        return False
    try:
        res = subprocess.run(["arduino-cli", "board", "listall", "sparkfun"],
                             capture_output=True, text=True, check=False)
        if fqbn in res.stdout:
            print(f"[+] Pinned Board FQBN ({fqbn}): INSTALLED & RECOGNIZED")
            return True
        else:
            print(f"[-] Pinned Board FQBN ({fqbn}): NOT RECOGNIZED")
            print("    Run 'python scripts/install_board_support.py' to overlay board definitions.")
            return False
    except Exception as e:
        print(f"[!] Error checking board list: {e}")
        return False

def check_hardware_manifest(manifest_path):
    if not os.path.exists(manifest_path):
        print(f"[-] Hardware manifest missing: {manifest_path}")
        return False
    print(f"[+] Hardware manifest present: {manifest_path}")
    # Inspect manifest for unconfirmed markers
    with open(manifest_path, "r", encoding="utf-8") as f:
        content = f.read()
    unconfirmed_count = content.count("UNCONFIRMED") + content.count("TODO(HARDWARE)")
    print(f"    Hardware safety gate: {unconfirmed_count} unconfirmed hardware items flagged.")
    return True

def main():
    print("=" * 60)
    print("Patient Monitor Toolchain and Environment Verification")
    print("=" * 60)

    all_ok = True

    # Python
    py_ok = sys.version_info >= (3, 8)
    if py_ok:
        print(f"[+] Python: {sys.version.split()[0]} [{sys.executable}]")
    else:
        print(f"[-] Python 3.8+ required, found: {sys.version}")
        all_ok = False

    # Make
    make_cmd = "make" if shutil.which("make") else ("mingw32-make" if shutil.which("mingw32-make") else None)
    if make_cmd:
        check_command(make_cmd, "GNU Make")
    else:
        print("[-] GNU Make (make or mingw32-make): NOT FOUND")
        all_ok = False

    # Host C++ compiler
    cxx_cmd = "g++" if shutil.which("g++") else ("clang++" if shutil.which("clang++") else None)
    if cxx_cmd:
        check_command(cxx_cmd, "Host C++ Compiler")
    else:
        print("[-] Host C++ Compiler (g++ or clang++): NOT FOUND")
        all_ok = False

    # Arduino CLI
    cli_ok, cli_ver = check_command("arduino-cli", "Arduino CLI", "version")
    if not cli_ok:
        all_ok = False
    else:
        # Check Core and Board
        board_ok = check_arduino_board("adafruit:nrf52:sparkfunnrf52840mini")
        if not board_ok:
            all_ok = False

    # Hardware Manifest
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    manifest = os.path.join(repo_root, "config", "hardware_manifest.yaml")
    manifest_ok = check_hardware_manifest(manifest)
    if not manifest_ok:
        all_ok = False

    print("=" * 60)
    if all_ok:
        print("ALL CRITICAL CHECKS PASSED: Environment ready for build and test.")
        return 0
    else:
        print("SOME CHECKS FAILED: Review items above before compiling.")
        return 1

if __name__ == "__main__":
    sys.exit(main())
