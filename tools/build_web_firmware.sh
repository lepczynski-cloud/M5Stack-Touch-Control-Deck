#!/bin/sh
# Build the verified firmware and prepare the optional static website for deployment.
# Does not modify the device firmware source.
set -eu

cd "$(dirname "$0")/.."

if ! command -v python3 >/dev/null 2>&1; then
    echo "ERROR: Python 3 is required by the build environment." >&2
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

# Never publish stale firmware if the new build fails.
rm -f web/firmware/m5stack-touch-control-deck.factory.bin web/firmware/m5stack-touch-control-deck.update.bin
rm -f web/firmware/manifest-update.json

# Reuse the photo already included in the repository without duplicating it in source control.
if [ -f images/m5stack-touch-control-deck.jpg ]; then
    cp images/m5stack-touch-control-deck.jpg web/assets/m5stack-touch-control-deck.jpg
else
    echo 'WARNING: images/m5stack-touch-control-deck.jpg is missing; the website will try the public GitHub URL.' >&2
fi

printf '%s\n' '[3/4] Building M5Stack CoreS3 SE firmware...'
"$VENV/bin/python" -m platformio run -e cores3_se

printf '%s\n' '[4/4] Merging bootloader, partitions and firmware for ESP Web Tools...'
"$VENV/bin/python" tools/package_web_firmware.py

test -s web/firmware/m5stack-touch-control-deck.update.bin || {
    echo 'ERROR: The app-only firmware update image is missing.' >&2
    exit 1
}
test -s web/firmware/m5stack-touch-control-deck.factory.bin || {
    echo 'ERROR: The firmware installer image is missing.' >&2
    exit 1
}

echo 'SUCCESS: web/ contains the current firmware and website ready for deployment.'
