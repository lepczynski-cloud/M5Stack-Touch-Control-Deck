#include "ConfigManager.h"

#include <algorithm>

#include "TimeManager.h"
#include "UserConfig.h"

namespace {
constexpr char NVS_NAMESPACE[] = "m5deck";
}

void ConfigManager::begin() {
    loadDefaults();

    Preferences preferences;
    if (!preferences.begin(NVS_NAMESPACE, true)) {
        sanitize();
        return;
    }

    config_.wifiEnabled = preferences.getBool("wifi_en", config_.wifiEnabled);
    config_.wifiSsid = preferences.getString("ssid", config_.wifiSsid);
    config_.wifiPassword = preferences.getString("wifi_pw", config_.wifiPassword);
    config_.brightness = preferences.getUChar("bright", config_.brightness);
    config_.secondTimezoneIndex = preferences.getUChar("tz2", config_.secondTimezoneIndex);
    config_.standbyEnabled = preferences.getBool("stby_en", config_.standbyEnabled);
    config_.standbyTimeoutMinutes = preferences.getUShort("stby_min", config_.standbyTimeoutMinutes);

    config_.apSsid = preferences.getString("ap_ssid", config_.apSsid);
    config_.apPassword = preferences.getString("ap_pw", config_.apPassword);
    config_.webUsername = preferences.getString("web_user", config_.webUsername);
    config_.webPassword = preferences.getString("web_pw", config_.webPassword);

    for (size_t index = 0; index < config_.apps.size(); ++index) {
        const String labelKey = "a" + String(index) + "n";
        const String urlKey = "a" + String(index) + "u";
        config_.apps[index].label = preferences.getString(labelKey.c_str(), config_.apps[index].label);
        config_.apps[index].url = preferences.getString(urlKey.c_str(), config_.apps[index].url);
    }

    preferences.end();
    sanitize();
}

bool ConfigManager::save() {
    sanitize();

    Preferences preferences;
    if (!preferences.begin(NVS_NAMESPACE, false)) {
        return false;
    }

    bool ok = true;
    ok &= preferences.putBool("wifi_en", config_.wifiEnabled) > 0;
    ok &= preferences.putString("ssid", config_.wifiSsid) > 0 || config_.wifiSsid.isEmpty();
    ok &= preferences.putString("wifi_pw", config_.wifiPassword) > 0 || config_.wifiPassword.isEmpty();
    ok &= preferences.putUChar("bright", config_.brightness) > 0;
    ok &= preferences.putUChar("tz2", config_.secondTimezoneIndex) > 0 || config_.secondTimezoneIndex == 0;
    ok &= preferences.putBool("stby_en", config_.standbyEnabled) > 0;
    ok &= preferences.putUShort("stby_min", config_.standbyTimeoutMinutes) > 0;

    ok &= preferences.putString("ap_ssid", config_.apSsid) > 0;
    ok &= preferences.putString("ap_pw", config_.apPassword) > 0;
    ok &= preferences.putString("web_user", config_.webUsername) > 0;
    ok &= preferences.putString("web_pw", config_.webPassword) > 0;

    for (size_t index = 0; index < config_.apps.size(); ++index) {
        const String labelKey = "a" + String(index) + "n";
        const String urlKey = "a" + String(index) + "u";
        ok &= preferences.putString(labelKey.c_str(), config_.apps[index].label) > 0;
        ok &= preferences.putString(urlKey.c_str(), config_.apps[index].url) > 0;
    }

    preferences.end();
    return ok;
}

void ConfigManager::resetToDefaults(bool saveNow) {
    Preferences preferences;
    if (preferences.begin(NVS_NAMESPACE, false)) {
        preferences.clear();
        preferences.end();
    }

    loadDefaults();
    sanitize();

    if (saveNow) {
        save();
    }
}

DeviceConfig& ConfigManager::data() {
    return config_;
}

const DeviceConfig& ConfigManager::data() const {
    return config_;
}

void ConfigManager::loadDefaults() {
    config_.wifiEnabled = UserDefaults::WIFI_ENABLED;
    config_.wifiSsid = UserDefaults::WIFI_SSID;
    config_.wifiPassword = UserDefaults::WIFI_PASSWORD;
    config_.brightness = UserDefaults::DISPLAY_BRIGHTNESS;
    config_.secondTimezoneIndex = UserDefaults::SECOND_TIMEZONE_INDEX;
    config_.standbyEnabled = UserDefaults::STANDBY_ENABLED;
    config_.standbyTimeoutMinutes = UserDefaults::STANDBY_TIMEOUT_MINUTES;

    config_.apSsid = UserDefaults::AP_SSID;
    config_.apPassword = UserDefaults::AP_PASSWORD;
    config_.webUsername = UserDefaults::WEB_USERNAME;
    config_.webPassword = UserDefaults::WEB_PASSWORD;

    for (size_t index = 0; index < config_.apps.size(); ++index) {
        config_.apps[index].label = UserDefaults::APP_LABELS[index];
        config_.apps[index].url = UserDefaults::APP_URLS[index];
    }
}

void ConfigManager::sanitize() {
    config_.wifiSsid = limitLength(config_.wifiSsid, 32);
    config_.wifiPassword = limitLength(config_.wifiPassword, 63);

    config_.brightness = std::max<uint8_t>(20, config_.brightness);
    if (config_.secondTimezoneIndex >= TimeManager::TIMEZONE_COUNT) {
        config_.secondTimezoneIndex = UserDefaults::SECOND_TIMEZONE_INDEX;
    }
    config_.standbyTimeoutMinutes = std::max<uint16_t>(
        1,
        std::min<uint16_t>(60, config_.standbyTimeoutMinutes)
    );

    config_.apSsid = limitLength(config_.apSsid, 32);
    config_.apPassword = limitLength(config_.apPassword, 63);
    config_.webUsername = limitLength(config_.webUsername, 24);
    config_.webPassword = limitLength(config_.webPassword, 63);

    config_.apSsid.trim();
    config_.webUsername.trim();

    if (config_.apSsid.isEmpty()) {
        config_.apSsid = UserDefaults::AP_SSID;
    }
    if (config_.apPassword.length() < 8) {
        config_.apPassword = UserDefaults::AP_PASSWORD;
    }
    if (config_.webUsername.isEmpty()) {
        config_.webUsername = UserDefaults::WEB_USERNAME;
    }
    if (config_.webPassword.isEmpty()) {
        config_.webPassword = UserDefaults::WEB_PASSWORD;
    }

    for (size_t index = 0; index < config_.apps.size(); ++index) {
        config_.apps[index].label = limitLength(config_.apps[index].label, 23);
        config_.apps[index].url = limitLength(config_.apps[index].url, 159);
        config_.apps[index].label.trim();
        config_.apps[index].url.trim();

        if (config_.apps[index].label.isEmpty()) {
            config_.apps[index].label = UserDefaults::APP_LABELS[index];
        }
        if (config_.apps[index].url.isEmpty()) {
            config_.apps[index].url = UserDefaults::APP_URLS[index];
        }
    }
}

String ConfigManager::limitLength(const String& value, size_t maxLength) {
    if (value.length() <= maxLength) {
        return value;
    }
    return value.substring(0, maxLength);
}
