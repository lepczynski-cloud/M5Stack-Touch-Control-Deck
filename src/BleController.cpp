#include "BleController.h"

#include <Esp.h>
#include <NimBLEDevice.h>

#include "UserConfig.h"

BleController::BleController()
    : keyboard_(
          UserDefaults::BLE_DEVICE_NAME,
          UserDefaults::BLE_MANUFACTURER,
          100
      ) {}

void BleController::begin(uint8_t initialBatteryLevel) {
    keyboard_.setDelay(12);
    keyboard_.setBatteryLevel(initialBatteryLevel);
    keyboard_.begin();
}

void BleController::loop(uint8_t batteryLevel) {
    const uint32_t now = millis();
    if (now - lastBatteryUpdateMillis_ >= 60000UL) {
        keyboard_.setBatteryLevel(batteryLevel);
        lastBatteryUpdateMillis_ = now;
    }
}

bool BleController::isConnected() const {
    return const_cast<BleKeyboard&>(keyboard_).isConnected();
}

void BleController::setHostOS(HostOS host) {
    hostOs_ = host;
}

HostOS BleController::hostOS() const {
    return hostOs_;
}

String BleController::bondedPeerIdentity() const {
    // Biblioteka trzyma jedna relacje BLE. Zwracamy adres powiazania,
    // a nie sam stan 'connected', aby nie pytac po kazdym reconnect.
    if (!isConnected() || NimBLEDevice::getNumBonds() != 1) {
        return "";
    }
    return String(NimBLEDevice::getBondedAddress(0).toString().c_str());
}

bool BleController::isSafeBrowserUrl(const String& url) {
    if (!(url.startsWith("https://") || url.startsWith("http://"))) return false;
    // Adres pochodzi z lokalnego portalu. Poniewaz symulujemy klawiature,
    // nie wysylamy znakow sterujacych ani cudzyslowow do pola Run/Alt+F2.
    // Polskie/spacjowane URL trzeba najpierw zakodowac procentowo.
    for (size_t i = 0; i < url.length(); ++i) {
        const char c = url.charAt(i);
        const bool asciiLetter = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
        const bool asciiDigit = c >= '0' && c <= '9';
        const bool common = c == ':' || c == '/' || c == '.' || c == '-' || c == '_' ||
            c == '?' || c == '&' || c == '=' || c == '%' || c == '+' || c == '~' ||
            c == '#' || c == '@' || c == '!' || c == ',' || c == ';';
        if (!(asciiLetter || asciiDigit || common)) return false;
    }
    return true;
}

bool BleController::runGnomeCommand(const String& command) {
    if (!ready()) return false;
    // GNOME: Alt+F2 uruchamia aplikacje i polecenia bez otwierania terminala.
    chord({KEY_LEFT_ALT, KEY_F2});
    delay(550);
    keyboard_.print(command);
    delay(UserDefaults::URL_SUBMIT_DELAY_MS);
    keyboard_.write(KEY_RETURN);
    return true;
}

bool BleController::openMacApp(const String& appName) {
    if (!ready()) return false;
    // Spotlight: Cmd+Space, nazwa systemowej aplikacji, Enter.
    chord({KEY_LEFT_GUI, static_cast<uint8_t>(' ')});
    delay(550);
    keyboard_.print(appName);
    delay(700);
    keyboard_.write(KEY_RETURN);
    return true;
}

bool BleController::openUrl(const String& url) {
    if (!ready() || !isSafeBrowserUrl(url)) return false;

    if (hostOs_ == HostOS::MacOS) {
        // Otwieramy Safari przez Spotlight, nastepnie Cmd+L i URL.
        // Jest to przewidywalne bez aplikacji towarzyszacej; nie narzucamy
        // obslugi domyslnej przegladarki, ktorej HID nie potrafi ustalic.
        if (!openMacApp("Safari")) return false;
        delay(1100);
        chord({KEY_LEFT_GUI, static_cast<uint8_t>('l')});
        delay(180);
        keyboard_.print(url);
        delay(UserDefaults::URL_SUBMIT_DELAY_MS);
        keyboard_.write(KEY_RETURN);
        return true;
    }

    if (hostOs_ == HostOS::LinuxGnome) {
        // GNOME + xdg-utils: bezpieczny zestaw znakow URL, bez shella.
        return runGnomeCommand(String("xdg-open \"") + url + "\"");
    }

    // Windows: Win+R i adres w domyslnej przegladarce.
    chord({KEY_LEFT_GUI, static_cast<uint8_t>('r')});
    delay(UserDefaults::RUN_DIALOG_DELAY_MS);
    keyboard_.print(url);
    delay(UserDefaults::URL_SUBMIT_DELAY_MS);
    keyboard_.write(KEY_RETURN);
    return true;
}

bool BleController::mediaPrevious() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_PREVIOUS_TRACK);
    return true;
}

bool BleController::mediaPlayPause() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_PLAY_PAUSE);
    return true;
}

bool BleController::mediaNext() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_NEXT_TRACK);
    return true;
}

bool BleController::mediaMute() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_MUTE);
    return true;
}

bool BleController::mediaVolumeDown() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_VOLUME_DOWN);
    return true;
}

bool BleController::mediaVolumeUp() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_VOLUME_UP);
    return true;
}

bool BleController::openTaskManager() {
    if (hostOs_ == HostOS::MacOS) return openMacApp("Activity Monitor");
    if (hostOs_ == HostOS::LinuxGnome) return runGnomeCommand("gnome-system-monitor");
    return chord({KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_ESC});
}

bool BleController::lockWindows() {
    if (hostOs_ == HostOS::MacOS) {
        return chord({KEY_LEFT_CTRL, KEY_LEFT_GUI, static_cast<uint8_t>('q')});
    }
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('l')});
}

bool BleController::openSnippingTool() {
    if (hostOs_ == HostOS::MacOS) {
        return chord({KEY_LEFT_GUI, KEY_LEFT_SHIFT, static_cast<uint8_t>('5')});
    }
    if (hostOs_ == HostOS::LinuxGnome) {
        return chord({KEY_PRTSC});
    }
    return chord({KEY_LEFT_GUI, KEY_LEFT_SHIFT, static_cast<uint8_t>('s')});
}

bool BleController::showDesktop() {
    if (hostOs_ == HostOS::MacOS) {
        // macOS Mission Control, zamiast niepewnego HID Fn+F11.
        return chord({KEY_LEFT_CTRL, KEY_UP_ARROW});
    }
    if (hostOs_ == HostOS::LinuxGnome) {
        // GNOME Activities, zamiast Super+D, ktore nie jest uniwersalne.
        return chord({KEY_LEFT_GUI});
    }
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('d')});
}

bool BleController::openFileExplorer() {
    if (hostOs_ == HostOS::MacOS) {
        // Option+Cmd+Space otwiera okno szukania w Finderze.
        return chord({KEY_LEFT_ALT, KEY_LEFT_GUI, static_cast<uint8_t>(' ')});
    }
    if (hostOs_ == HostOS::LinuxGnome) return runGnomeCommand("nautilus");
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('e')});
}

bool BleController::switchWindow() {
    if (hostOs_ == HostOS::MacOS) return chord({KEY_LEFT_GUI, KEY_TAB}, 120);
    if (hostOs_ == HostOS::LinuxGnome) return chord({KEY_LEFT_GUI, KEY_TAB}, 120);
    return chord({KEY_LEFT_ALT, KEY_TAB}, 120);
}

void BleController::forgetBondAndRestart() {
    keyboard_.releaseAll();
    NimBLEDevice::deleteAllBonds();
    delay(600);
    ESP.restart();
}

bool BleController::ready() const {
    return isConnected();
}

bool BleController::chord(std::initializer_list<uint8_t> keys, uint32_t holdMs) {
    if (!ready()) {
        return false;
    }

    for (const uint8_t key : keys) {
        keyboard_.press(key);
    }
    delay(holdMs);
    keyboard_.releaseAll();
    return true;
}
