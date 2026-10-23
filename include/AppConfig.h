#pragma once

#include <Arduino.h>
#include <array>

// Bluetooth HID nie przekazuje systemu operacyjnego hosta.
// Uzytkownik sam wybiera docelowy zestaw skrotow.
enum class HostOS : uint8_t { Windows = 0, MacOS = 1, LinuxGnome = 2 };

// Bluetooth HID nie umożliwia ustalenia przeglądarki domyślnej na Macu.
// Wybrana przeglądarka uruchamiana jest jawnie przez Spotlight (Aplikacje).
enum class MacBrowser : uint8_t { Chrome = 0, Safari = 1, Firefox = 2, Brave = 3, Edge = 4 };

inline const char* macBrowserName(MacBrowser browser) {
    switch (browser) {
        case MacBrowser::Safari: return "Safari";
        case MacBrowser::Firefox: return "Firefox";
        case MacBrowser::Brave: return "Brave Browser";
        case MacBrowser::Edge: return "Microsoft Edge";
        default: return "Google Chrome";
    }
}

inline const char* hostOsName(HostOS host) {
    switch (host) {
        case HostOS::MacOS: return "macOS";
        case HostOS::LinuxGnome: return "Linux GNOME";
        default: return "Windows";
    }
}

struct AppShortcut {
    String label;
    String url;
};

struct DeviceConfig {
    bool wifiEnabled = true;
    String wifiSsid;
    String wifiPassword;
    uint8_t brightness = 150;
    uint8_t secondTimezoneIndex = 0;
    HostOS hostOs = HostOS::Windows;
    // Ostatnie zaakceptowane powiazanie BLE (adres identyfikacyjny hosta).
    // Sluzy wylacznie do wyswietlenia pytania o system po nowym parowaniu.
    String pairedHostId;
    // Wybierana w portalu AP przegladarka macOS (domyslnie Chrome).
    MacBrowser macBrowser = MacBrowser::Chrome;
    bool standbyEnabled = true;
    uint16_t standbyTimeoutMinutes = 5;

    String apSsid;
    String apPassword;
    String webUsername;
    String webPassword;

    std::array<AppShortcut, 6> apps;
};
