"""Instaluje brakujace zaleznosci narzedzi PlatformIO w jego interpreterze Python."""

from __future__ import annotations

import importlib
import subprocess
import sys

Import("env")  # type: ignore[name-defined]  # Udostepniane przez SCons/PlatformIO.


def ensure_module(module_name: str, package_spec: str) -> None:
    """Instaluje pakiet tylko wtedy, gdy nie jest dostepny dla PlatformIO."""
    try:
        importlib.import_module(module_name)
        return
    except ImportError:
        print(f"[zaleznosci] Brakuje {package_spec}. Instalowanie...")

    subprocess.check_call(
        [
            sys.executable,
            "-m",
            "pip",
            "install",
            "--disable-pip-version-check",
            package_spec,
        ]
    )
    importlib.invalidate_caches()
    importlib.import_module(module_name)


# Nowsze wydania tool-esptoolpy importuja intelhex podczas kazdego builda,
# chociaz PlatformIO nie zawsze instaluje ten pakiet automatycznie.
ensure_module("intelhex", "intelhex==2.3.0")
