#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "AppConfig.h"

class ConfigManager {
public:
    void begin();
    bool save();
    void resetToDefaults(bool saveNow = true);

    DeviceConfig& data();
    const DeviceConfig& data() const;

private:
    DeviceConfig config_;

    void loadDefaults();
    void sanitize();
    static String limitLength(const String& value, size_t maxLength);
};
