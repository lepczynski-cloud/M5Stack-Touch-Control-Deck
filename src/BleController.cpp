#include "BleController.h"

#include <Esp.h>
#include <NimBLEDevice.h>

#include "UserConfig.h"

namespace {
constexpr uint16_t MAC_HID_KEY_DELAY_MS = 20;
constexpr uint16_t DEFAULT_HID_KEY_DELAY_MS = 12;
constexpr uint32_t MAC_SPOTLIGHT_FOCUS_MS = 650;
constexpr uint32_t MAC_APP_RESULTS_MS = 1100;
constexpr uint32_t MAC_BROWSER_RESULTS_MS = 1000;
constexpr uint32_t MAC_BROWSER_ACTIVATION_MS = 2200;
constexpr uint32_t MAC_ADDRESS_BAR_FOCUS_MS = 220;
}

BleController::BleController()
    : keyboard_(
          UserDefaults::BLE_DEVICE_NAME,
          UserDefaults::BLE_MANUFACTURER,
          100
      ) {}

void BleController::begin(uint8_t initialBatteryLevel) {
    keyboard_.setDelay(hostOs_ == HostOS::MacOS ? MAC_HID_KEY_DELAY_MS : DEFAULT_HID_KEY_DELAY_MS);
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
    // Nieco wolniejsze wysylanie znakow na macOS pomaga przy zapytaniach Spotlight.
    keyboard_.setDelay(hostOs_ == HostOS::MacOS ? MAC_HID_KEY_DELAY_MS : DEFAULT_HID_KEY_DELAY_MS);
}

HostOS BleController::hostOS() const {
    return hostOs_;
}

void BleController::setMacBrowser(MacBrowser browser) {
    macBrowser_ = browser;
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

bool BleController::openMacLauncherOnly() {
    if (!ready()) return false;

    // Wysylamy rzeczywista sekwencje klawisza i modyfikatora: najpierw
    // naciskamy Cmd, potem spacje, a nastepnie zwalniamy je osobno.
    // Skrót moze byc przemapowany w ustawieniach macOS (Spotlight).
    keyboard_.releaseAll();
    delay(75);
    keyboard_.press(KEY_LEFT_GUI);
    delay(65);
    keyboard_.press(static_cast<uint8_t>(' '));
    delay(170);
    keyboard_.release(static_cast<uint8_t>(' '));
    delay(65);
    keyboard_.release(KEY_LEFT_GUI);
    delay(MAC_SPOTLIGHT_FOCUS_MS);
    return ready();
}

bool BleController::openMacSpotlightQuery(const String& query, uint32_t resultsDelayMs) {
    if (!openMacLauncherOnly() || query.isEmpty()) return false;
    keyboard_.print(query);
    delay(resultsDelayMs);
    if (!ready()) return false;
    keyboard_.write(KEY_RETURN);
    return true;
}

bool BleController::openMacApp(const String& appName) {
    // Pozostawiamy metode potwierdzona na Macu: zwykle Spotlight dla
    // Findera i Monitora aktywnosci.
    return openMacSpotlightQuery(appName, MAC_APP_RESULTS_MS);
}

bool BleController::openMacBrowser() {
    if (!openMacLauncherOnly()) return false;
    // macOS Tahoe: Cmd+1 ogranicza wyniki Spotlight do aplikacji.
    // Dzięki temu plik z nazwa ChatGPT nie moze zostac wybrany jako URL.
    if (!chord({KEY_LEFT_GUI, static_cast<uint8_t>('1')})) return false;
    delay(220);
    if (!ready()) return false;
    keyboard_.print(String(macBrowserName(macBrowser_)));
    delay(MAC_BROWSER_RESULTS_MS);
    if (!ready()) return false;
    keyboard_.write(KEY_RETURN);
    delay(MAC_BROWSER_ACTIVATION_MS);
    return ready();
}

bool BleController::openUrl(const String& url) {
    if (!ready() || !isSafeBrowserUrl(url)) return false;

    if (hostOs_ == HostOS::MacOS) {
        // Nie zatwierdzamy URL jako wyniku wyszukiwania Spotlight: na
        // macOS Tahoe "ChatGPT" moze otworzyc dokument o tej nazwie.
        // Najpierw otwieramy wskazana przez uzytkownika przegladarke
        // w Spotlight (filtr Aplikacje), a potem jej pasek adresu.
        if (!openMacBrowser()) return false;
        if (!chord({KEY_LEFT_GUI, static_cast<uint8_t>('l')})) return false;
        delay(MAC_ADDRESS_BAR_FOCUS_MS);
        if (!ready()) return false;
        keyboard_.print(url);
        delay(UserDefaults::URL_SUBMIT_DELAY_MS);
        if (!ready()) return false;
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
        // F11 na zewnetrznej klawiaturze HID: "Show Desktop" w macOS.
        // Na wbudowanej klawiaturze MacBooka Apple opisuje go jako Fn+F11.
        // Jezeli uzytkownik zmienil przypisanie, powinien ustawic F11 w
        // System Settings > Keyboard > Keyboard Shortcuts > Mission Control.
        return chord({KEY_F11});
    }
    if (hostOs_ == HostOS::LinuxGnome) {
        // GNOME Activities, zamiast Super+D, ktore nie jest uniwersalne.
        return chord({KEY_LEFT_GUI});
    }
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('d')});
}

bool BleController::openFileExplorer() {
    if (hostOs_ == HostOS::MacOS) {
        // Spotlight znajduje Finder i aktywuje normalne okno plikow,
        // niezaleznie od tego, ktora aplikacja jest teraz na pierwszym planie.
        return openMacApp("Finder");
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
