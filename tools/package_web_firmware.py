#!/usr/bin/env python3
"""Package a previously built CoreS3 SE project for ESP Web Tools.

No third-party Python modules are needed by this wrapper. Espressif esptool
is supplied by the existing PlatformIO installation after a normal build.
"""
from __future__ import annotations

import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BUILD = ROOT / ".pio" / "build" / "cores3_se"
OUTPUT = ROOT / "web" / "firmware" / "m5stack-touch-control-deck.factory.bin"
MANIFEST = ROOT / "web" / "firmware" / "manifest.json"


def pio_homes() -> list[Path]:
    result = []
    for env_var in ("PLATFORMIO_CORE_DIR", "PLATFORMIO_HOME_DIR"):
        if os.environ.get(env_var):
            result.append(Path(os.environ[env_var]))
    result.append(Path.home() / ".platformio")
    if os.environ.get("USERPROFILE"):
        result.append(Path(os.environ["USERPROFILE"]) / ".platformio")
    return list(dict.fromkeys(result))


def find_package(name: str) -> Path:
    for core in pio_homes():
        candidate = core / "packages" / name
        if candidate.exists():
            return candidate
    raise FileNotFoundError(f"PlatformIO package {name} was not found. Run tools/build_windows.bat first.")


def pick(*paths: Path) -> Path:
    for path in paths:
        if path.is_file():
            return path
    raise FileNotFoundError("Required build image not found:\n" + "\n".join(str(p) for p in paths))


def firmware_version() -> str:
    source = (ROOT / "include" / "UserConfig.h").read_text(encoding="utf-8")
    match = re.search(r'FIRMWARE_VERSION\[\]\s*=\s*"([0-9]+\.[0-9]+\.[0-9]+)"', source)
    if not match:
        raise ValueError("Cannot detect firmware version in include/UserConfig.h")
    return match.group(1)


def validate_image(label: str, path: Path, expected_magic: bytes) -> None:
    with path.open("rb") as stream:
        actual = stream.read(len(expected_magic))
    if actual != expected_magic:
        raise ValueError(f"{label} does not look like a valid ESP32-S3 image: {path}")


def main() -> int:
    if not BUILD.is_dir():
        raise FileNotFoundError("No PlatformIO build. Run tools/build_windows.bat first.")
    framework = find_package("framework-arduinoespressif32")
    esptool = find_package("tool-esptoolpy") / "esptool.py"
    if not esptool.is_file():
        raise FileNotFoundError(f"esptool.py not found: {esptool}")

    sdk_s3 = framework / "tools" / "sdk" / "esp32s3" / "bin"
    bootloader = pick(
        BUILD / "bootloader.bin",
        sdk_s3 / "bootloader_qio_80m.bin",
        sdk_s3 / "bootloader_dio_80m.bin",
        sdk_s3 / "bootloader_dio_40m.bin",
    )
    partitions = pick(BUILD / "partitions.bin")
    boot_app0 = pick(framework / "tools" / "partitions" / "boot_app0.bin")
    app = pick(BUILD / "firmware.bin")

    validate_image("Bootloader", bootloader, b"\xe9")
    validate_image("App", app, b"\xe9")
    validate_image("Partition table", partitions, b"\xaa\x50")

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    command = [
        sys.executable, str(esptool), "--chip", "esp32s3", "merge_bin",
        "-o", str(OUTPUT),
        "--flash_mode", "dio", "--flash_freq", "40m", "--flash_size", "16MB",
        "0x0", str(bootloader),
        "0x8000", str(partitions),
        "0xe000", str(boot_app0),
        "0x10000", str(app),
    ]
    print("Packaging these exact files:")
    for name, file in (("bootloader", bootloader), ("partitions", partitions),
                       ("boot_app0", boot_app0), ("firmware", app)):
        print(f"  {name}: {file}")
    print("Merge command:", " ".join(command), flush=True)
    subprocess.run(command, cwd=ROOT, check=True)

    size = OUTPUT.stat().st_size
    if not 128 * 1024 < size < 16 * 1024 * 1024:
        OUTPUT.unlink(missing_ok=True)
        raise ValueError(f"Unexpected merged firmware size: {size}")
    validate_image("Merged factory image", OUTPUT, b"\xe9")

    version = firmware_version()
    manifest = {
        "name": "M5Stack Touch Control Deck",
        "version": version,
        "new_install_prompt_erase": True,
        "builds": [{"chipFamily": "ESP32-S3", "parts": [
            {"path": OUTPUT.name, "offset": 0}
        ]}]
    }
    MANIFEST.write_text(json.dumps(manifest, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    checksum = hashlib.sha256(OUTPUT.read_bytes()).hexdigest()
    print(f"READY: {OUTPUT.relative_to(ROOT)}")
    print(f"Firmware version: {version}")
    print(f"Firmware size: {size:,} bytes")
    print(f"SHA-256: {checksum}")
    print("The firmware image and manifest are ready for static website deployment.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        raise SystemExit(1)
