# M5Stack Touch Control Deck

![M5Stack Touch Control Deck - wireless touchscreen controller for Windows, macOS and Linux](images/m5stack-touch-control-deck.jpg)

**Project website & browser installer (EN/PL):** [M5Stack-Touch-Control-Deck.lepczynski.it](https://m5stack-touch-control-deck.lepczynski.it/)

A wireless touchscreen controller based on **M5Stack CoreS3 SE**. It acts as a Bluetooth HID keyboard and media controller, with manually selectable shortcut profiles for **Windows 10/11, macOS and Linux (GNOME/Ubuntu)**. After new Bluetooth pairing, the device asks which operating system to control; no companion app is required on the host.

**Compatibility note:** Windows is the established baseline. macOS testing revealed a regression in v0.2.1: when the Spotlight shortcut failed, queries were typed into the active app. **v0.2.2 requires confirmation that Spotlight opened before sending the first query**. macOS shortcuts remain dependent on host settings; Linux GNOME still needs validation on your machine.

## Two ways to install the firmware

**1. Directly from your browser, no local compilation (recommended).** Visit the [web installer](https://m5stack-touch-control-deck.lepczynski.it/), connect **M5Stack CoreS3 SE** with a USB-C data cable, then select the serial port in Chrome or Edge. Choose **Firmware update** for an existing device: it flashes only the application and can preserve settings **if you do NOT select "Erase device"** and the partition layout is unchanged. Choose **Factory install** for initial setup/reset: it flashes the whole image and **erases NVS settings**, including Wi-Fi and custom app buttons. No Python or PlatformIO needed. **Do not use this installer with other ESP32 models.**

**2. Build and upload from source, with full customization.** Download the repository, connect the device via USB-C, and run `tools\upload_windows.bat` on Windows. The script installs its toolchain, compiles the source, and uploads the firmware. Use this method if you want to modify behavior, icons, or shortcuts. More details: [docs/INSTALLATION.md](docs/INSTALLATION.md).

After flashing, pair **M5 Control Deck** over Bluetooth and select **Windows**, **macOS** or **Linux GNOME** on the touchscreen. You can change this later from **Connections** on the device or through its local AP portal at `http://192.168.4.1`. No companion software is required.

**Detailed build tutorial (in Polish):** [Forbot forum article](https://forbot.pl/forum/topic/26182-m5stack-control-deck-bezprzewodowy-panel-sterowania-komputerem-na-cores3-se-bez-lutowania/). You can translate the page in your browser.

**Full build video:** [M5Stack Touch Control Deck on YouTube](https://www.youtube.com/watch?v=Jz1xCBi-Q1o).

Polish documentation: [README.md](README.md).

This project extends the idea from [ESP32 PC Control Deck](https://github.com/lepczynski-cloud/ESP32-PC-Control-Deck). The earlier device is shown in this [YouTube video](https://www.youtube.com/watch?v=16eqHU4j9Fk).

The device interface and source-code comments are in Polish. This English guide explains installation, configuration and the main customization points.

## Main features

- six pages navigated with vertical swipe gestures,
- Poland clock plus a configurable second timezone, HKT by default,
- optional NTP synchronization and RTC-based offline operation,
- six configurable website shortcuts,
- touchscreen media controls,
- optional Scroll Unit support for volume and Play/Pause,
- OS-aware shortcuts for Windows, macOS or Linux GNOME,
- local configuration portal hosted by the device,
- Wi-Fi, Bluetooth and battery status icons,
- animated startup, standby, wake and shutdown,
- power menu and inactivity standby prompt.

## Compatibility

Three BLE HID keyboard profiles are available: **Windows 10/11**, **macOS** and **Linux GNOME/Ubuntu**. BLE HID does **not** reliably identify the host OS. When a **new Bluetooth bond** is detected, the device asks you to select the host OS. Your choice is stored in non-volatile memory, so reconnecting to the same computer **does not trigger the prompt again**.

Change the profile any time on the **Connections** touchscreen or in the local AP portal (`http://192.168.4.1`). No reflashing or Bluetooth re-pairing is needed to change profiles.

**Limitations:** macOS uses **Spotlight** (`Cmd+Space`) to open URLs in the default browser or launch applications. Before the **first query**, the deck sends only the shortcut and asks **"Did Spotlight open?"** on its own display (the on-device UI is in Polish: **NIE / TAK**). Select **NIE** if it did not; the deck will NOT type into your active editor. Check **System Settings → Keyboard → Keyboard Shortcuts → Spotlight** and try the shortcut manually. Selecting **TAK** permits the query and later attempts without prompting. This is a manual confirmation, **not** remote focus detection; changing host shortcuts later can still break the sequence. `Cmd+F3` (Show Desktop) depends on macOS settings. Linux targets **GNOME/Ubuntu** with `xdg-open`, `nautilus` and `gnome-system-monitor`.

## Hardware used in this build

### Required

1. [M5Stack CoreS3 SE, K128-SE](https://botland.com.pl/seria-core/25332-m5stack-cores3-se-modul-deweloperski-esp32-s3-m5stack-k128-se-6972934175304.html)
2. a data-capable USB-C cable for the initial upload

### Optional

- [M5Stack Battery Module 13.2, 1500 mAh, M120](https://botland.com.pl/seria-core/21459-m5stack-battery-module-132-nakladka-z-akumulatorem-1500mah-do-modulow-deweloperskich-m5stack-core-m5stack-m120-6972934173546.html)
- [M5Stack Scroll Unit, U186](https://botland.com.pl/moduly-unit-moduly-rozszerzen/25428-scroll-enkoder-obrotowy-z-przyciskiem-modul-rozszerzen-unit-m5stack-u186-6972934175441.html)

Without the battery module, the deck can run from USB-C power. Without the Scroll Unit, every touchscreen page still works and the media controls remain available on the display.

Connect the Scroll Unit to Grove Port A. The firmware uses I2C address `0x40`, SDA GPIO 2 and SCL GPIO 1 through `M5.Ex_I2C`.

## Optional 3D-printed enclosure

My build uses [M5Stack Core 2 case 45° angle w/Unit holder](https://makerworld.com/en/models/3128546-m5stack-core-2-case-45deg-angle-w-unit-holder#profileId-3530280), created by MakerWorld user Gigi. It is not my model, and its files are not included in this repository.

The model was designed for the M5Stack Core family and an external Unit holder. It fits my CoreS3 SE, Battery Module 13.2 and Scroll Unit build very well. An enclosure is optional, and you can use another model or no enclosure at all.

## Pages

### Clock

Shows Poland time, a second timezone and the date. A blue background indicates active Wi-Fi synchronization, while a gray background indicates offline RTC operation.

### Applications

The default 2 x 3 layout contains ChatGPT, Claude, Gemini, Canva, Admin Toolkit and Botland. On Windows the deck uses **Win+R**; on macOS it uses **Cmd+Space** and types the URL only after the first Spotlight confirmation; on GNOME it uses **Alt+F2 → xdg-open**.

Names and URLs can be changed in the local portal or `include/UserConfig.h`. Use `http://` or `https://` URLs with safe ASCII characters and percent-encode spaces as `%20`.

### Media

Previous track, Play/Pause, next track, mute, volume down and volume up. When the optional Scroll Unit is connected, rotation changes volume and a click sends Play/Pause.

### System shortcuts

| Action | Windows | macOS | Linux GNOME |
|---|---|---|---|
| System monitor | Ctrl+Shift+Esc | Spotlight → Activity Monitor | Alt+F2 → gnome-system-monitor |
| Lock screen | Win+L | Ctrl+Cmd+Q | Super+L |
| Screenshot | Win+Shift+S | Shift+Cmd+5 | Print Screen |
| Desktop / overview | Win+D | Cmd+F3 (Show Desktop) | Super (Activities) |
| Files | Win+E | Spotlight → Finder | Alt+F2 → nautilus |
| Switch applications | Alt+Tab | Cmd+Tab | Super+Tab |

Labels change when you select a host OS. macOS uses `Cmd+F3` for Show Desktop (requires the corresponding macOS keyboard shortcut), and GNOME uses Activities rather than a universal Show Desktop shortcut.

### Settings

Brightness, home Wi-Fi, configuration access point and the second timezone.

### Connections

Bluetooth state, **Windows/macOS/Linux GNOME selector**, new pairing, local AP address and firmware version.

## Windows source upload (alternative to the web installer)

Use this option to compile the source or make your own changes. For a ready-to-use installation, choose the browser installer described above. This source-based method needs Python 3.9 or newer and Internet access on the first run.

1. Connect the CoreS3 SE with a data-capable USB-C cable.
2. Run:

```bat
tools\upload_windows.bat
```

The script creates a local `.pio-venv`, installs PlatformIO, builds the firmware, rejects Bluetooth virtual COM ports and uploads through the verified Espressif USB port.

You can provide the port explicitly:

```bat
tools\upload_windows.bat COM7
```

If the board is not detected, hold **RST** for 2 to 3 seconds until the green LED turns on, release it and run the script again.

See [docs/INSTALLATION.md](docs/INSTALLATION.md) for details.

## Bluetooth pairing

1. On the host computer (Windows, macOS or GNOME Linux), open Bluetooth settings and pair **M5 Control Deck**.
2. On a new pairing, the touchscreen asks you to choose **Windows**, **macOS** or **Linux**.
3. The selected shortcut profile is saved and reused when reconnecting to the same host.
4. Change the profile later from the **Connections** screen or the AP portal, with no reflashing. You can also choose through the portal while a new pairing prompt is visible.

### Switching computers

The firmware keeps one active BLE bond.

1. Open the **Connections** page and start a new pairing.
2. Confirm removal of the current bond.
3. Remove `M5 Control Deck` from the Bluetooth device list on the previous computer.
4. Pair it with the new computer.

Windows can keep a stale pairing record after the deck removes its local bond. When returning to a previously used computer, remove the existing `M5 Control Deck` entry there, start new pairing on the deck and add it again.

## Configuration portal

When no home Wi-Fi credentials are stored, the device starts this access point:

```text
SSID: M5-Control-Deck
AP password: m5control
Address: http://192.168.4.1
Username: admin
Panel password: admin123
```

The AP can also be enabled manually from the Settings page. The portal changes the **target operating system (Windows, macOS or Linux GNOME)**, Wi-Fi credentials, brightness, the second timezone, standby settings, AP credentials, portal credentials, and all six app labels and URLs. Saving the form reboots the device.

Change the default passwords after the first setup. The portal uses local HTTP, so disable the AP when configuration is complete.

## Source customization

Default values are stored in `include/UserConfig.h`. The file contains Wi-Fi and AP settings, brightness, timezone, application tiles, Scroll Unit direction and standby defaults.

Values already saved in NVS override source defaults. Use **Factory reset** in the portal after changing default values in code.

The source files use Polish comments. The most important modules are:

- `src/BleController.cpp`: BLE HID keyboard, three host shortcut profiles, media keys and URLs,
- `src/UserInterface.cpp`: pages, gestures, icons, standby and power menu,
- `src/NetworkPortal.cpp`: Wi-Fi, AP and web configuration,
- `src/TimeManager.cpp`: NTP, RTC and timezones,
- `src/ScrollController.cpp`: optional encoder support,
- `src/SystemAnimations.cpp`: startup, standby, wake and shutdown animations.

Detailed customization examples are in [docs/CUSTOMIZATION.md](docs/CUSTOMIZATION.md).

## Current limitations

- Windows 10/11 is the established baseline. macOS has been tested on a real Mac but version 0.2.1 was observed to type queries into the active editor; v0.2.2 adds a first-use Spotlight check and still requires device testing. Linux GNOME requires host-specific verification.
- Typing URLs relies on compatible ASCII key mappings (Polish Programmer works for the original Windows setup); other keyboard layouts may need changes.
- macOS website launching uses Spotlight to open the **default browser**; GNOME requires `xdg-open` and a working Alt+F2 command launcher.
- The host should be unlocked for opening URLs and launching applications.
- BLE HID cannot read CPU, RAM, GPU or temperatures; telemetry would require a separate host application.
- One active Bluetooth bond is stored at a time.

## Project website

The `web/` directory contains the optional bilingual project website and browser flasher. It is not required to compile the firmware locally or run the device. See [web/README.md](web/README.md).

## License

MIT. See [LICENSE](LICENSE).
