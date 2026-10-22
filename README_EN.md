# M5Stack Touch Control Deck

![M5Stack Touch Control Deck - wireless touchscreen controller for Windows](images/m5stack-touch-control-deck.jpg)

**Project website & browser installer (EN/PL):** [M5Stack-Touch-Control-Deck.lepczynski.it](https://m5stack-touch-control-deck.lepczynski.it/)

A wireless touchscreen control deck for Windows 10 and Windows 11. The M5Stack CoreS3 SE acts as a Bluetooth Low Energy keyboard and media controller, so everyday features do not require a companion application on the computer.

## Two ways to install the firmware

**1. Directly from your browser, no local compilation (recommended).** Visit the [web installer](https://m5stack-touch-control-deck.lepczynski.it/), connect **M5Stack CoreS3 SE** using a data-capable USB-C cable, choose **Install firmware**, and select the device's USB port. The site flashes the latest **published, precompiled firmware**. You don't need to install Python, PlatformIO, or an IDE. Use a compatible desktop browser (for example Chrome or Edge) over HTTPS. Installing firmware may erase previous device settings. **Do not flash another ESP32 board using this installer.**

**2. Build and upload from source, with full customization.** Download the repository, connect the device via USB-C, and run `tools\upload_windows.bat` on Windows. The script installs its toolchain, compiles the source, and uploads the firmware. Use this method if you want to modify behavior, icons, or shortcuts. More details: [docs/INSTALLATION.md](docs/INSTALLATION.md).

After flashing, pair **M5 Control Deck** through Bluetooth in Windows 10/11. No companion software is needed for everyday PC control.

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
- useful Windows 10/11 shortcuts,
- local configuration portal hosted by the device,
- Wi-Fi, Bluetooth and battery status icons,
- animated startup, standby, wake and shutdown,
- power menu and inactivity standby prompt.

## Compatibility

This first public release supports Windows 10 and Windows 11 only. Website shortcuts use `Win + R`, and the system page sends Windows-specific keyboard combinations.

The device is a standard BLE HID keyboard. BLE HID does not provide a reliable host operating-system identifier, so this release uses one explicit shortcut set for Windows.

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

The default 2 x 3 layout contains ChatGPT, Claude, Gemini, Canva, Admin Toolkit and Botland. A tap sends `Win + R`, types the configured URL and presses Enter.

Names and URLs can be changed in the local portal or in `include/UserConfig.h`.

### Media

Previous track, Play/Pause, next track, mute, volume down and volume up. When the optional Scroll Unit is connected, rotation changes volume and a click sends Play/Pause.

### Windows

- Task Manager: `Ctrl + Shift + Esc`
- lock: `Win + L`
- Snipping Tool selection: `Win + Shift + S`
- show desktop: `Win + D`
- File Explorer: `Win + E`
- switch window: `Alt + Tab`

### Settings

Brightness, home Wi-Fi, configuration access point and the second timezone.

### Connections

Bluetooth state, new pairing, local portal details, Scroll Unit status and firmware version.

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

1. Open **Settings > Bluetooth & devices > Add device > Bluetooth** in Windows.
2. Select **M5 Control Deck**.
3. The Bluetooth icon on the deck turns green after the connection is established.

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

The AP can also be enabled manually from the Settings page. The portal changes Wi-Fi credentials, brightness, the second timezone, standby settings, AP credentials, portal credentials, and all six application labels and URLs.

Change the default passwords after the first setup. The portal uses local HTTP, so disable the AP when configuration is complete.

## Source customization

Default values are stored in `include/UserConfig.h`. The file contains Wi-Fi and AP settings, brightness, timezone, application tiles, Scroll Unit direction and standby defaults.

Values already saved in NVS override source defaults. Use **Factory reset** in the portal after changing default values in code.

The source files use Polish comments. The most important modules are:

- `src/BleController.cpp`: BLE keyboard, media keys, Windows shortcuts and URLs,
- `src/UserInterface.cpp`: pages, gestures, icons, standby and power menu,
- `src/NetworkPortal.cpp`: Wi-Fi, AP and web configuration,
- `src/TimeManager.cpp`: NTP, RTC and timezones,
- `src/ScrollController.cpp`: optional encoder support,
- `src/SystemAnimations.cpp`: startup, standby, wake and shutdown animations.

Detailed customization examples are in [docs/CUSTOMIZATION.md](docs/CUSTOMIZATION.md).

## Current limitations

- Windows 10 and Windows 11 only.
- The expected keyboard layout is Polish Programmer.
- The PC must be unlocked for `Win + R` website launching.
- BLE HID cannot read CPU, RAM, GPU or temperatures. Those values require a separate Windows companion application.
- One active Bluetooth bond is stored at a time.

## Project website

The `web/` directory contains the optional bilingual project website and browser flasher. It is not required to compile the firmware locally or run the device. See [web/README.md](web/README.md).

## License

MIT. See [LICENSE](LICENSE).
