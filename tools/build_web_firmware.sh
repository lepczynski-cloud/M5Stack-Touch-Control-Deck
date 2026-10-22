#!/bin/sh
# Build firmware automatically during each Cloudflare Pages deployment.
# Cloudflare config: build command = sh tools/build_web_firmware.sh
#                   output directory = web
set -eu

cd "$(dirname "$0")/.."

if ! command -v python3 >/dev/null 2>&1; then
    echo "ERROR: Python 3 is required by the Cloudflare build." >&2
    exit 1
fi

VENV=".web-build-venv"
if [ ! -x "$VENV/bin/python" ]; then
    echo "[1/4] Creating build-only Python environment..."
    python3 -m venv "$VENV"
fi

printf '%s\n' '[2/4] Installing pinned PlatformIO and esptool dependencies...'
"$VENV/bin/python" -m pip install --disable-pip-version-check \
    'platformio==6.2.0' 'intelhex==2.3.0'

# Never publish an old local image after a failed build.
rm -f web/firmware/m5stack-touch-control-deck.factory.bin

printf '%s\n' '[3/4] Building M5Stack CoreS3 SE firmware...'
"$VENV/bin/python" -m platformio run -e cores3_se

printf '%s\n' '[4/4] Merging bootloader, partitions and firmware for ESP Web Tools...'
"$VENV/bin/python" tools/package_web_firmware.py

test -s web/firmware/m5stack-touch-control-deck.factory.bin || {
    echo 'ERROR: The firmware installer image is missing.' >&2
    exit 1
}

echo 'SUCCESS: Cloudflare can now deploy the web/ directory with the firmware.'
