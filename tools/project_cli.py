#!/usr/bin/env python3
"""Budowanie, wgrywanie i diagnostyka projektu w Windows."""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
import time
from collections.abc import Sequence
from pathlib import Path
from typing import Optional

ROOT = Path(__file__).resolve().parents[1]
VENV_DIR = ROOT / ".pio-venv"
ENVIRONMENT = "cores3_se"
ESPRESSIF_VID = 0x303A

REQUIRED_FILES = [
    "platformio.ini",
    "README.md",
    "README_EN.md",
    "LICENSE",
    ".gitignore",
    ".gitattributes",
    "docs/CUSTOMIZATION.md",
    "docs/INSTALLATION.md",
    "docs/TROUBLESHOOTING.md",
    "include/AppConfig.h",
    "include/UserConfig.h",
    "include/BleController.h",
    "include/ConfigManager.h",
    "include/NetworkPortal.h",
    "include/ScrollController.h",
    "include/SystemAnimations.h",
    "include/TimeManager.h",
    "include/UserInterface.h",
    "src/main.cpp",
    "src/BleController.cpp",
    "src/ConfigManager.cpp",
    "src/NetworkPortal.cpp",
    "src/ScrollController.cpp",
    "src/SystemAnimations.cpp",
    "src/TimeManager.cpp",
    "src/UserInterface.cpp",
    "tools/project_cli.py",
    "tools/ensure_platformio_deps.py",
    "tools/build_windows.bat",
    "tools/upload_windows.bat",
    "tools/monitor_windows.bat",
]

EXPECTED_URLS = [
    "https://chatgpt.com/",
    "https://claude.ai/",
    "https://gemini.google.com/",
    "https://www.canva.com/",
    "https://admintoolkit.lepczynski.it/",
    "https://botland.com.pl/",
]

IGNORED_DIRECTORIES = {
    ".git",
    ".pio",
    ".pio-venv",
    ".idea",
    ".vscode",
    "__pycache__",
}


def run(
    command: list[str],
    *,
    check: bool = True,
    capture_output: bool = False,
) -> subprocess.CompletedProcess[str]:
    """Uruchamia polecenie w katalogu projektu i zachowuje czytelny log."""
    return subprocess.run(
        command,
        cwd=ROOT,
        check=check,
        text=True,
        capture_output=capture_output,
    )


def venv_python() -> Path:
    if os.name == "nt":
        return VENV_DIR / "Scripts" / "python.exe"
    return VENV_DIR / "bin" / "python"


def venv_platformio() -> Path:
    if os.name == "nt":
        return VENV_DIR / "Scripts" / "platformio.exe"
    return VENV_DIR / "bin" / "platformio"


def is_ignored(path: Path) -> bool:
    relative = path.relative_to(ROOT)
    return any(
        part in IGNORED_DIRECTORIES or part.startswith("cmake-build-")
        for part in relative.parts
    )


def strip_cpp_comments_and_strings(text: str) -> str:
    pattern = re.compile(
        r'("(?:\\.|[^"\\])*")|(\'(?:\\.|[^\'\\])*\')|(//[^\n]*)|(/\*.*?\*/)',
        re.DOTALL,
    )
    return pattern.sub(
        lambda match: "" if match.group(3) or match.group(4) else match.group(0),
        text,
    )


def balanced(text: str, opening: str, closing: str) -> bool:
    level = 0
    for character in text:
        if character == opening:
            level += 1
        elif character == closing:
            level -= 1
            if level < 0:
                return False
    return level == 0


def text_files() -> list[Path]:
    extensions = {".h", ".cpp", ".md", ".ini", ".py", ".bat", ".txt"}
    result: list[Path] = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or is_ignored(path):
            continue
        if path.suffix.lower() in extensions or path.name in {
            "LICENSE",
            ".gitignore",
            ".gitattributes",
        }:
            result.append(path)
    return result


def validate_project() -> None:
    """Sprawdza strukturę i najważniejsze elementy przed kompilacją."""
    print("[1/5] Sprawdzanie plikow projektu...", flush=True)
    failed = False

    for relative in REQUIRED_FILES:
        if not (ROOT / relative).is_file():
            print(f"BLAD: brakuje pliku {relative}", file=sys.stderr)
            failed = True

    if failed:
        raise SystemExit(1)

    config = (ROOT / "include/UserConfig.h").read_text(encoding="utf-8")
    source_files = sorted((ROOT / "src").glob("*.cpp"))
    header_files = sorted((ROOT / "include").glob("*.h"))
    source = "\n".join(path.read_text(encoding="utf-8") for path in source_files)
    headers = "\n".join(path.read_text(encoding="utf-8") for path in header_files)
    platformio_ini = (ROOT / "platformio.ini").read_text(encoding="utf-8")
    dependency_script = (ROOT / "tools/ensure_platformio_deps.py").read_text(
        encoding="utf-8"
    )

    for url in EXPECTED_URLS:
        if url not in config:
            print(f"BLAD: brakuje domyslnego adresu {url}", file=sys.stderr)
            failed = True

    required_markers = [
        'AP_ADDRESS(192, 168, 4, 1)',
        '"M5-Control-Deck"',
        '"M5 Control Deck"',
        'FIRMWARE_VERSION[] = "0.1.0"',
        "openSnippingTool",
        "KEY_MEDIA_PLAY_PAUSE",
        "deleteAllBonds",
        "Screen::Connections",
        "M5Canvas",
        "TAP_MAX_MOVEMENT",
        "SCROLL_REVERSE_DIRECTION",
        "M5.Ex_I2C",
        "ScrollController",
        "mediaVolumeUp",
        "drawWifiStatusIcon",
        "drawBluetoothStatusIcon",
        "SystemAnimations::startup",
        "M5.Power.powerOff",
        "M5.Display.sleep",
        "STANDBY_TIMEOUT_MINUTES",
    ]
    combined = config + "\n" + headers + "\n" + source
    for marker in required_markers:
        if marker not in combined:
            print(f"BLAD: brakuje wymaganego elementu: {marker}", file=sys.stderr)
            failed = True

    if re.search(r'WIFI_PASSWORD\[\]\s*=\s*"[^\"]+"', config):
        print(
            "BLAD: domyslne haslo domowego Wi-Fi powinno pozostac puste",
            file=sys.stderr,
        )
        failed = True

    if "extra_scripts = pre:tools/ensure_platformio_deps.py" not in platformio_ini:
        print(
            "BLAD: PlatformIO nie uruchamia naprawy zaleznosci esptool",
            file=sys.stderr,
        )
        failed = True

    if "intelhex==2.3.0" not in dependency_script or "sys.executable" not in dependency_script:
        print(
            "BLAD: skrypt PlatformIO nie instaluje intelhex we wlasciwym interpreterze",
            file=sys.stderr,
        )
        failed = True

    for path in source_files + header_files:
        text = path.read_text(encoding="utf-8")
        cleaned = strip_cpp_comments_and_strings(text)
        if not balanced(cleaned, "{", "}"):
            print(
                f"BLAD: niezrownowazone nawiasy klamrowe w {path.relative_to(ROOT)}",
                file=sys.stderr,
            )
            failed = True
        if "\t" in text:
            print(
                f"BLAD: znaleziono tabulator w {path.relative_to(ROOT)}",
                file=sys.stderr,
            )
            failed = True

    for path in text_files():
        text = path.read_text(encoding="utf-8", errors="replace")
        if ("TO" + "DO") in text or ("FIX" + "ME") in text:
            print(
                f"BLAD: plik {path.relative_to(ROOT)} zawiera oznaczenie niedokonczonej pracy",
                file=sys.stderr,
            )
            failed = True

    if failed:
        raise SystemExit(1)

    print("Walidacja zakonczona powodzeniem.")
    print(
        f"Sprawdzono {len(REQUIRED_FILES)} wymaganych plikow "
        f"i {len(EXPECTED_URLS)} adresow URL."
    )


def ensure_platformio() -> tuple[Path, Path]:
    python_exe = venv_python()
    pio_exe = venv_platformio()

    if not python_exe.exists():
        print("[2/5] Tworzenie lokalnego srodowiska Python...", flush=True)
        run([sys.executable, "-m", "venv", str(VENV_DIR)])

    if not pio_exe.exists():
        print("[2/5] Instalowanie PlatformIO w lokalnym srodowisku...", flush=True)
        run(
            [
                str(python_exe),
                "-m",
                "pip",
                "install",
                "--disable-pip-version-check",
                "platformio>=6.1,<7",
            ]
        )
    else:
        print("[2/5] PlatformIO jest gotowe.", flush=True)

    return python_exe, pio_exe


def python_module_available(python_exe: Path, module_name: str) -> bool:
    result = run(
        [str(python_exe), "-c", f"import {module_name}"],
        check=False,
        capture_output=True,
    )
    return result.returncode == 0


def ensure_tool_dependencies(python_exe: Path) -> None:
    """Naprawia zależności narzędzi PlatformIO pomijane przez niektóre paczki."""
    print("[3/5] Sprawdzanie zaleznosci narzedzi ESP32...", flush=True)

    if not python_module_available(python_exe, "intelhex"):
        print("Brakuje intelhex==2.3.0. Instalowanie...", flush=True)
        run(
            [
                str(python_exe),
                "-m",
                "pip",
                "install",
                "--disable-pip-version-check",
                "intelhex==2.3.0",
            ]
        )

    if not python_module_available(python_exe, "intelhex"):
        print("BLAD: Nie udalo sie przygotowac zaleznosci intelhex.", file=sys.stderr)
        raise SystemExit(3)

    print("Zaleznosci narzedzi sa gotowe.", flush=True)


def build_firmware(pio_exe: Path) -> None:
    print("[4/5] Kompilowanie firmware...", flush=True)
    run([str(pio_exe), "run", "-e", ENVIRONMENT])


def is_bluetooth_port(port: object) -> bool:
    description = str(getattr(port, "description", "") or "").lower()
    hardware_id = str(getattr(port, "hwid", "") or "").lower()
    return "bthenum" in hardware_id or "bluetooth" in description


def is_esp32_port(port: object) -> bool:
    description = str(getattr(port, "description", "") or "").lower()
    manufacturer = str(getattr(port, "manufacturer", "") or "").lower()
    hardware_id = str(getattr(port, "hwid", "") or "").lower()
    vid = getattr(port, "vid", None)

    if is_bluetooth_port(port):
        return False

    return (
        vid == ESPRESSIF_VID
        or "vid:pid=303a:" in hardware_id
        or "vid_303a" in hardware_id
        or "usb jtag/serial" in description
        or "usb serial/jtag" in description
        or "esp32-s3" in description
        or "espressif" in manufacturer
    )


def print_detected_ports(ports: Sequence[object]) -> None:
    if not ports:
        print("System nie zwrocil zadnych portow szeregowych.", file=sys.stderr)
        return

    print("Wykryte porty:", file=sys.stderr)
    for port in ports:
        description = str(getattr(port, "description", "") or "brak opisu")
        marker = "Bluetooth" if is_bluetooth_port(port) else "USB/inny"
        print(f"  {port.device}: {description} [{marker}]", file=sys.stderr)


def detect_port_process(arguments: list[str]) -> int:
    """Działa w lokalnym Pythonie PlatformIO, który zawiera pyserial."""
    from serial.tools import list_ports

    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("port", nargs="?")
    parser.add_argument("--wait", type=int, default=0)
    args = parser.parse_args(arguments)

    deadline = time.monotonic() + max(0, args.wait)
    last_ports: list[object] = []
    multiple_candidates: list[object] = []

    while True:
        ports = list(list_ports.comports())
        last_ports = ports

        if args.port:
            requested = args.port.lower()
            match = next(
                (port for port in ports if str(port.device).lower() == requested),
                None,
            )
            if match is not None:
                if not is_esp32_port(match):
                    print(
                        f"BLAD: {match.device} nie wyglada na port USB CoreS3 SE.",
                        file=sys.stderr,
                    )
                    print(f"Opis: {match.description}", file=sys.stderr)
                    print(
                        "Nie wybieraj portu opisanego jako link Bluetooth.",
                        file=sys.stderr,
                    )
                    return 5
                print(match.device)
                return 0
        else:
            candidates = [port for port in ports if is_esp32_port(port)]
            if len(candidates) == 1:
                print(candidates[0].device)
                return 0
            if len(candidates) > 1:
                multiple_candidates = candidates

        if time.monotonic() >= deadline:
            break
        time.sleep(0.5)

    if args.port:
        print(
            f"BLAD: Port {args.port} nie istnieje lub urzadzenie jest odlaczone.",
            file=sys.stderr,
        )
        print_detected_ports(last_ports)
        return 4

    if multiple_candidates:
        print("BLAD: Znaleziono wiecej niz jeden port USB Espressif:", file=sys.stderr)
        for port in multiple_candidates:
            print(f"  {port.device}: {port.description}", file=sys.stderr)
        print(
            "Podaj port jawnie, np. tools\\upload_windows.bat COM7",
            file=sys.stderr,
        )
        return 3

    print("BLAD: Nie znaleziono portu USB CoreS3 SE.", file=sys.stderr)
    print("1. Podlacz przewod USB-C obslugujacy dane.", file=sys.stderr)
    print("2. Przytrzymaj RST przez 2-3 sekundy do zapalenia zielonej diody.", file=sys.stderr)
    print("3. Pusc RST i uruchom skrypt ponownie.", file=sys.stderr)
    print("Porty Bluetooth COM nie sluza do wgrywania firmware.", file=sys.stderr)
    print_detected_ports(last_ports)
    return 2


def detect_port(python_exe: Path, requested_port: Optional[str], wait_seconds: int) -> str:
    print("[5/5] Wyszukiwanie portu USB CoreS3 SE...", flush=True)
    command = [
        str(python_exe),
        str(Path(__file__).resolve()),
        "_detect-port",
        "--wait",
        str(wait_seconds),
    ]
    if requested_port:
        command.append(requested_port)

    result = run(command, check=False, capture_output=True)
    if result.stderr:
        print(result.stderr, file=sys.stderr, end="")
    if result.returncode != 0:
        raise SystemExit(result.returncode)

    port = result.stdout.strip().splitlines()[-1] if result.stdout.strip() else ""
    if not port:
        print("BLAD: Nie udalo sie ustalic portu USB.", file=sys.stderr)
        raise SystemExit(2)
    return port


def command_build(_: argparse.Namespace) -> int:
    validate_project()
    python_exe, pio_exe = ensure_platformio()
    ensure_tool_dependencies(python_exe)
    build_firmware(pio_exe)
    print("\nSUKCES: Firmware zostal skompilowany.")
    return 0


def command_upload(args: argparse.Namespace) -> int:
    validate_project()
    python_exe, pio_exe = ensure_platformio()
    ensure_tool_dependencies(python_exe)
    build_firmware(pio_exe)
    port = detect_port(python_exe, args.port, args.wait)

    print(f"Wykryto CoreS3 SE na porcie {port}.")
    print("Wgrywanie firmware...")
    result = run(
        [
            str(pio_exe),
            "run",
            "-e",
            ENVIRONMENT,
            "-t",
            "upload",
            "--upload-port",
            port,
        ],
        check=False,
    )
    if result.returncode != 0:
        print("\nBLAD: Wgrywanie nie powiodlo sie.", file=sys.stderr)
        print(
            "Przytrzymaj RST przez 2-3 sekundy do zapalenia zielonej diody, "
            "pusc przycisk i uruchom skrypt ponownie.",
            file=sys.stderr,
        )
        return result.returncode

    print(f"\nSUKCES: Firmware zostal wgrany na {port}.")
    return 0


def command_monitor(args: argparse.Namespace) -> int:
    validate_project()
    python_exe, pio_exe = ensure_platformio()
    port = detect_port(python_exe, args.port, args.wait)
    print(f"Monitor portu {port}. Zakoncz przez Ctrl+C.")
    return run(
        [
            str(pio_exe),
            "device",
            "monitor",
            "--baud",
            "115200",
            "--port",
            port,
        ],
        check=False,
    ).returncode


def create_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Budowanie i wgrywanie M5Stack Touch Control Deck w Windows."
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    build_parser = subparsers.add_parser("build", help="Sprawdz i skompiluj projekt.")
    build_parser.set_defaults(handler=command_build)

    upload_parser = subparsers.add_parser("upload", help="Skompiluj i wgraj firmware.")
    upload_parser.add_argument("port", nargs="?", help="Opcjonalny port, np. COM7.")
    upload_parser.add_argument(
        "--wait",
        type=int,
        default=20,
        help="Czas oczekiwania na port USB w sekundach.",
    )
    upload_parser.set_defaults(handler=command_upload)

    monitor_parser = subparsers.add_parser(
        "monitor",
        help="Uruchom monitor portu szeregowego.",
    )
    monitor_parser.add_argument("port", nargs="?", help="Opcjonalny port, np. COM7.")
    monitor_parser.add_argument(
        "--wait",
        type=int,
        default=10,
        help="Czas oczekiwania na port USB w sekundach.",
    )
    monitor_parser.set_defaults(handler=command_monitor)

    return parser


def main() -> int:
    if sys.version_info < (3, 9):
        print("BLAD: Wymagany jest Python 3.9 lub nowszy.", file=sys.stderr)
        return 1

    if len(sys.argv) >= 2 and sys.argv[1] == "_detect-port":
        return detect_port_process(sys.argv[2:])

    parser = create_parser()
    args = parser.parse_args()
    try:
        return int(args.handler(args))
    except subprocess.CalledProcessError as error:
        return error.returncode or 1
    except KeyboardInterrupt:
        print("\nPrzerwano przez uzytkownika.")
        return 130


if __name__ == "__main__":
    raise SystemExit(main())
