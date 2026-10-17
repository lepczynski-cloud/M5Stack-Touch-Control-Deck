#pragma once

#include <Arduino.h>
#include <M5Unified.h>

#include "ConfigManager.h"

struct TimezoneOption {
    const char* label;
    const char* shortLabel;
    const char* posix;
};

class TimeManager {
public:
    static constexpr size_t TIMEZONE_COUNT = 8;

    explicit TimeManager(ConfigManager& configManager);

    void begin();
    void loop(bool wifiConnected);
    bool syncNow(uint32_t timeoutMs = 4500);

    bool hasValidTime() const;
    bool wasSyncedThisBoot() const;
    uint32_t lastSyncMillis() const;

    String polishTime(bool withSeconds = false) const;
    String secondTime(bool withSeconds = false) const;
    String polishDate() const;
    String secondTimezoneName() const;
    String secondTimezoneShortName() const;

    static const TimezoneOption& timezone(size_t index);

private:
    ConfigManager& configManager_;
    bool syncedThisBoot_ = false;
    uint32_t lastSyncMillis_ = 0;
    uint32_t nextSyncAttemptMillis_ = 0;

    static String formatTime(const char* posixTimezone, bool withSeconds);
    static String formatDate(const char* posixTimezone);
    static bool epochLooksValid(time_t epoch);
};
