# Patient Monitor - Setup Script for Windows PowerShell
# Pinned versions:
# - Adafruit nRF52 Core: 1.7.0
# - Target FQBN: adafruit:nrf52:sparkfunnrf52840mini

$ErrorActionPreference = "Stop"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "Setting up Patient Monitor Development Environment (Windows)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# 1. Check Python
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    Write-Error "Python 3 is required. Please install Python 3.8+."
}
Write-Host "[+] Python: $($python.Source)" -ForegroundColor Green

# 2. Check or suggest Host Compiler and Make
$cxx = Get-Command g++, clang++, cl -ErrorAction SilentlyContinue
if (-not $cxx) {
    Write-Warning "Host C++ compiler (g++ / clang++) not found in PATH."
    Write-Warning "To install via winget: winget install BrechtSanders.WinLibs.POSIX.UCRT"
} else {
    Write-Host "[+] Host C++ compiler: $($cxx.Source)" -ForegroundColor Green
}

$make = Get-Command make, mingw32-make -ErrorAction SilentlyContinue
if (-not $make) {
    Write-Warning "GNU Make not found in PATH. Ensure 'make' or 'mingw32-make' is accessible."
} else {
    Write-Host "[+] Make tool: $($make.Source)" -ForegroundColor Green
}

# 3. Check or install Arduino CLI
$arduinoCli = Get-Command arduino-cli -ErrorAction SilentlyContinue
if (-not $arduinoCli) {
    # Check default install path
    if (Test-Path "C:\Program Files\Arduino CLI\arduino-cli.exe") {
        $env:PATH += ";C:\Program Files\Arduino CLI"
        $arduinoCli = Get-Command arduino-cli -ErrorAction SilentlyContinue
    }
}

if (-not $arduinoCli) {
    Write-Host "[*] Arduino CLI not found. Attempting install via winget..." -ForegroundColor Yellow
    try {
        winget install --id ArduinoSA.CLI --exact --accept-package-agreements --accept-source-agreements
        if (Test-Path "C:\Program Files\Arduino CLI\arduino-cli.exe") {
            $env:PATH += ";C:\Program Files\Arduino CLI"
        }
    } catch {
        Write-Error "Could not install Arduino CLI automatically. Please install arduino-cli."
    }
}

# 4. Configure Board Index and Install Adafruit nRF52 Core 1.7.0
$adafruitIndex = "https://adafruit.github.io/arduino-board-index/package_adafruit_index.json"
Write-Host "[*] Updating Arduino index for Adafruit nRF52..." -ForegroundColor Yellow
arduino-cli core update-index --additional-urls $adafruitIndex

Write-Host "[*] Installing pinned core adafruit:nrf52@1.7.0..." -ForegroundColor Yellow
arduino-cli core install adafruit:nrf52@1.7.0 --additional-urls $adafruitIndex

# 5. Overlay SparkFun Board Support
Write-Host "[*] Installing SparkFun Pro nRF52840 Mini board definition..." -ForegroundColor Yellow
python scripts/install_board_support.py

# 6. Verify Toolchain
Write-Host "[*] Verifying toolchain..." -ForegroundColor Yellow
python scripts/verify_toolchain.py

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "Setup complete!" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan
