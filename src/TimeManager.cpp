#include "TimeManager.h"

#include <time.h>

#include "UserConfig.h"

namespace {
constexpr char POLAND_TIMEZONE[] = "CET-1CEST,M3.5.0/2,M10.5.0/3";
constexpr time_t MIN_VALID_EPOCH = 1704067200;  // 2024-01-01 00:00:00 UTC
constexpr uint32_t REGULAR_SYNC_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL;
constexpr uint32_t RETRY_SYNC_INTERVAL_MS = 60UL * 1000UL;

const TimezoneOption TIMEZONES[TimeManager::TIMEZONE_COUNT] = {
    {"Hongkong", "HKT", "HKT-8"},
    {"UTC", "UTC", "UTC0"},
    {"Londyn", "LON", "GMT0BST,M3.5.0/1,M10.5.0/2"},
    {"Nowy Jork", "NYC", "EST5EDT,M3.2.0/2,M11.1.0/2"},
    {"Los Angeles", "LAX", "PST8PDT,M3.2.0/2,M11.1.0/2"},
    {"Singapur", "SGT", "SGT-8"},
    {"Tokio", "JST", "JST-9"},
    {"Sydney", "SYD", "AEST-10AEDT,M10.1.0/2,M4.1.0/3"},
};
}

TimeManager::TimeManager(ConfigManager& configManager)
    : configManager_(configManager) {}

void TimeManager::begin() {
    // RTC jest przechowywany w UTC. Dzieki temu zmiana strefy czasowej nie
    // wymaga przestawiania zegara sprzetowego.
    if (M5.Rtc.isEnabled()) {
        M5.Rtc.setSystemTimeFromRtc();
    }

    if (hasValidTime()) {
        nextSyncAttemptMillis_ = 2000;
    } else {
        nextSyncAttemptMillis_ = 500;
    }
}

void TimeManager::loop(bool wifiConnected) {
    if (!wifiConnected) {
        return;
    }

    const uint32_t nowMillis = millis();
    if (static_cast<int32_t>(nowMillis - nextSyncAttemptMillis_) < 0) {
        return;
    }

    if (syncNow()) {
        nextSyncAttemptMillis_ = nowMillis + REGULAR_SYNC_INTERVAL_MS;
    } else {
        nextSyncAttemptMillis_ = nowMillis + RETRY_SYNC_INTERVAL_MS;
    }
}

bool TimeManager::syncNow(uint32_t timeoutMs) {
    configTime(
        0,
        0,
        UserDefaults::NTP_SERVER_1,
        UserDefaults::NTP_SERVER_2,
        UserDefaults::NTP_SERVER_3
    );

    struct tm utcTime {};
    if (!getLocalTime(&utcTime, timeoutMs)) {
        return false;
    }

    const time_t now = time(nullptr);
    if (!epochLooksValid(now)) {
        return false;
    }

    // Po udanej synchronizacji aktualizujemy RTC, aby zegar dzialal takze
    // po wylaczeniu Wi-Fi lub po restarcie urzadzenia.
    if (M5.Rtc.isEnabled()) {
        struct tm utcCopy {};
        gmtime_r(&now, &utcCopy);
        M5.Rtc.setDateTime(&utcCopy);
    }

    syncedThisBoot_ = true;
    lastSyncMillis_ = millis();
    return true;
}

bool TimeManager::hasValidTime() const {
    return epochLooksValid(time(nullptr));
}

bool TimeManager::wasSyncedThisBoot() const {
    return syncedThisBoot_;
}

uint32_t TimeManager::lastSyncMillis() const {
    return lastSyncMillis_;
}

String TimeManager::polishTime(bool withSeconds) const {
    return formatTime(POLAND_TIMEZONE, withSeconds);
}

String TimeManager::secondTime(bool withSeconds) const {
    return formatTime(timezone(configManager_.data().secondTimezoneIndex).posix, withSeconds);
}

String TimeManager::polishDate() const {
    return formatDate(POLAND_TIMEZONE);
}

String TimeManager::secondTimezoneName() const {
    return timezone(configManager_.data().secondTimezoneIndex).label;
}

String TimeManager::secondTimezoneShortName() const {
    return timezone(configManager_.data().secondTimezoneIndex).shortLabel;
}

const TimezoneOption& TimeManager::timezone(size_t index) {
    if (index >= TIMEZONE_COUNT) {
        index = 0;
    }
    return TIMEZONES[index];
}

String TimeManager::formatTime(const char* posixTimezone, bool withSeconds) {
    const time_t now = time(nullptr);
    if (!epochLooksValid(now)) {
        return "--:--";
    }

    setenv("TZ", posixTimezone, 1);
    tzset();

    struct tm localTime {};
    localtime_r(&now, &localTime);

    char buffer[16] = {};
    strftime(buffer, sizeof(buffer), withSeconds ? "%H:%M:%S" : "%H:%M", &localTime);
    return String(buffer);
}

String TimeManager::formatDate(const char* posixTimezone) {
    const time_t now = time(nullptr);
    if (!epochLooksValid(now)) {
        return "Brak ustawionego czasu";
    }

    setenv("TZ", posixTimezone, 1);
    tzset();

    struct tm localTime {};
    localtime_r(&now, &localTime);

    static constexpr const char* WEEKDAYS[] = {
        "Niedziela", "Poniedzialek", "Wtorek", "Sroda",
        "Czwartek", "Piatek", "Sobota"
    };

    char dateBuffer[24] = {};
    strftime(dateBuffer, sizeof(dateBuffer), "%d.%m.%Y", &localTime);
    return String(WEEKDAYS[localTime.tm_wday]) + ", " + dateBuffer;
}

bool TimeManager::epochLooksValid(time_t epoch) {
    return epoch >= MIN_VALID_EPOCH;
}
