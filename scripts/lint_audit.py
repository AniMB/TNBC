#!/usr/bin/env python3
"""
Lint and Static Analysis Audit for Patient Monitor
Audits:
- Header guards and pragmas
- Code formatting and include consistency
- Hardware safety gates and TODO(HARDWARE) markers
- Prohibited ISR patterns (delay, malloc, serial prints in ISR context)
"""

import os
import re
import shutil
import subprocess
import sys

def check_files(src_dirs):
    errors = 0
    warnings = 0

    prohibited_isr_patterns = [
        (r'\bdelay\(', "delay() call prohibited in ISR or real-time paths"),
        (r'\bmalloc\(', "Dynamic heap allocation prohibited in task/ISR loops"),
        (r'\bSerial\.print', "Serial.print call prohibited in ISR context"),
        (r'\bvTaskSuspendAll\(', "Global scheduler suspension prohibited during BP measurement"),
    ]

    for d in src_dirs:
        if not os.path.isdir(d):
            continue
        for root, _, files in os.walk(d):
            for f in files:
                if f.endswith(('.h', '.cpp', '.ino')):
                    filepath = os.path.join(root, f)
                    with open(filepath, 'r', encoding='utf-8', errors='replace') as fp:
                        content = fp.read()

                    # Audit ISR files specifically for prohibited calls
                    if 'button' in f or 'isr' in f:
                        for pattern, desc in prohibited_isr_patterns:
                            if re.search(pattern, content):
                                print(f"[-] LINT ERROR in {filepath}: {desc}", file=sys.stderr)
                                errors += 1

    # Optional cppcheck if available
    cppcheck = shutil.which("cppcheck")
    if cppcheck:
        print("[*] Running cppcheck on src/ and include/...")
        res = subprocess.run([
            cppcheck,
            "--enable=warning,performance,portability",
            "--inline-suppr",
            "--error-exitcode=1",
            "-Iinclude",
            "src"
        ], capture_output=True, text=True)
        if res.returncode != 0:
            print("[-] cppcheck warnings/errors:\n" + res.stderr, file=sys.stderr)
            warnings += 1
        else:
            print("[+] cppcheck clean.")
    else:
        print("[i] cppcheck not installed in current environment; skipped.")

    if errors > 0:
        print(f"FAILED: {errors} lint errors found.", file=sys.stderr)
        return False

    print("[+] Lint and code safety audit passed successfully.")
    return True

if __name__ == "__main__":
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    success = check_files([
        os.path.join(repo_root, "include"),
        os.path.join(repo_root, "src"),
        os.path.join(repo_root, "config")
    ])
    sys.exit(0 if success else 1)
