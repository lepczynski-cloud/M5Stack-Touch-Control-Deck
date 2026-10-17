#pragma once

#include <Arduino.h>
#include <array>

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
    bool standbyEnabled = true;
    uint16_t standbyTimeoutMinutes = 5;

    String apSsid;
    String apPassword;
    String webUsername;
    String webPassword;

    std::array<AppShortcut, 6> apps;
};
