#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#include "ConfigManager.h"
#include "BleController.h"
#include "TimeManager.h"

class NetworkPortal {
public:
    NetworkPortal(ConfigManager& configManager, TimeManager& timeManager, BleController& bleController);

    void begin();
    void loop();

    bool isWifiConnected() const;
    bool isWifiEnabled() const;
    bool isApActive() const;
    bool isWifiConnecting() const;

    String wifiSsid() const;
    String wifiIp() const;
    String apSsid() const;
    String apIp() const;
    String webUsername() const;

    void setWifiEnabled(bool enabled);
    void setApEnabled(bool enabled);
    void reconnectWifi();

private:
    ConfigManager& configManager_;
    TimeManager& timeManager_;
    BleController& bleController_;
    WebServer server_{80};
    DNSServer dnsServer_;

    bool routesRegistered_ = false;
    bool serverRunning_ = false;
    bool apActive_ = false;
    bool wifiAttempting_ = false;
    uint32_t wifiAttemptStartedMillis_ = 0;
    uint32_t nextWifiRetryMillis_ = 0;
    uint32_t lastPortalActivityMillis_ = 0;
    uint32_t pendingRestartMillis_ = 0;
    uint32_t pendingApStopMillis_ = 0;

    void startStation();
    void stopStation();
    void startAccessPoint();
    void stopAccessPoint();
    void applyWifiMode();

    void registerRoutes();
    bool authenticate();
    void markActivity();

    void handleRoot();
    void handleSave();
    void handleStatus();
    void handleRestart();
    void handleDisableAp();
    void handleFactoryReset();
    void handleNotFound();

    String buildPortalHtml() const;
    static String htmlEscape(const String& input);
    static String jsonEscape(const String& input);
    static String safeFormValue(const String& input, size_t maxLength);
    static bool isAllowedUrl(const String& url);
};
