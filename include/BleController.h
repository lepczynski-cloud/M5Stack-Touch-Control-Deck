#pragma once

#include <Arduino.h>
#include <NimBleKeyboard.h>
#include "AppConfig.h"
#include <initializer_list>

class BleController {
public:
    BleController();

    void begin(uint8_t initialBatteryLevel);
    void loop(uint8_t batteryLevel);

    bool isConnected() const;
    void setHostOS(HostOS host);
    HostOS hostOS() const;
    void setMacBrowser(MacBrowser browser);
    String bondedPeerIdentity() const;
    static bool isSafeBrowserUrl(const String& url);
    // Konfigurowalny zestaw skrotow. Linux oznacza srodowisko GNOME.

    bool openUrl(const String& url);

    bool mediaPrevious();
    bool mediaPlayPause();
    bool mediaNext();
    bool mediaMute();
    bool mediaVolumeDown();
    bool mediaVolumeUp();

    bool openTaskManager();
    bool lockWindows();
    bool openSnippingTool();
    bool showDesktop();
    bool openFileExplorer();
    bool switchWindow();

    void forgetBondAndRestart();

private:
    BleKeyboard keyboard_;
    uint32_t lastBatteryUpdateMillis_ = 0;
    HostOS hostOs_ = HostOS::Windows;
    MacBrowser macBrowser_ = MacBrowser::Chrome;

    bool runGnomeCommand(const String& command);
    bool openMacApp(const String& appName);
    bool openMacLauncherOnly();
    bool openMacSpotlightQuery(const String& query, uint32_t resultsDelayMs);
    bool openMacBrowser();

    bool ready() const;
    bool chord(std::initializer_list<uint8_t> keys, uint32_t holdMs = 70);
};
