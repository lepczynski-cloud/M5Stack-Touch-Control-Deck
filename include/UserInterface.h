#pragma once

#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>

#include "BleController.h"
#include "ConfigManager.h"
#include "NetworkPortal.h"
#include "ScrollController.h"
#include "TimeManager.h"

class UserInterface {
public:
    UserInterface(
        ConfigManager& configManager,
        TimeManager& timeManager,
        NetworkPortal& networkPortal,
        BleController& bleController,
        ScrollController& scrollUnit
    );

    void begin(bool scrollAvailable);
    void loop();
    void requestRedraw();

private:
    enum class Screen : uint8_t {
        Clock = 0,
        Apps,
        Media,
        Windows,
        Settings,
        Connections,
        Count
    };

    enum class Icon : uint8_t {
        Chat,
        Claude,
        Gemini,
        Design,
        AdminTools,
        Microchip,
        Previous,
        PlayPause,
        Next,
        Mute,
        VolumeDown,
        VolumeUp,
        TaskManager,
        Lock,
        Snip,
        Desktop,
        Folder,
        SwitchWindow
    };

    ConfigManager& configManager_;
    TimeManager& timeManager_;
    NetworkPortal& networkPortal_;
    BleController& bleController_;
    ScrollController& scrollUnit_;

    M5Canvas clockCardCanvas_;
    bool clockCardCanvasReady_ = false;

    Screen screen_ = Screen::Clock;
    bool scrollAvailable_ = false;
    bool dirty_ = true;
    bool statusBarDirty_ = true;
    bool clockValuesDirty_ = true;

    bool touchDown_ = false;
    int16_t touchStartX_ = 0;
    int16_t touchStartY_ = 0;
    int16_t touchLastX_ = 0;
    int16_t touchLastY_ = 0;
    int16_t touchMaxDistance_ = 0;
    uint32_t touchStartMillis_ = 0;

    int16_t lastEncoderValue_ = 0;
    bool encoderButtonDown_ = false;
    uint32_t encoderButtonDownMillis_ = 0;
    uint32_t lastEncoderPollMillis_ = 0;
    uint32_t lastEncoderDisconnectedToastMillis_ = 0;
    uint32_t lastEncoderClickMillis_ = 0;

    bool pairingConfirmationVisible_ = false;
    bool hostOsPromptVisible_ = false;

    String pendingHostPeerId_;
    bool powerMenuVisible_ = false;
    bool standbyPromptVisible_ = false;
    bool standby_ = false;
    bool suppressTouchUntilRelease_ = false;
    bool suppressEncoderClickUntilRelease_ = false;
    uint32_t lastInteractionMillis_ = 0;
    uint32_t standbyPromptStartedMillis_ = 0;
    uint8_t lastStandbyCountdownValue_ = 0xFF;
    bool lastStandbyBleConnected_ = false;

    String toastMessage_;
    uint32_t toastUntilMillis_ = 0;

    bool lastBleConnected_ = false;
    bool lastWifiConnected_ = false;
    bool lastApActive_ = false;
    bool lastCharging_ = false;
    int8_t lastBatteryLevel_ = -1;
    uint32_t lastStatusRefreshMillis_ = 0;
    uint32_t lastBatteryRefreshMillis_ = 0;
    uint32_t lastClockRefreshMillis_ = 0;
    uint32_t lastScrollLedColor_ = 0xFFFFFFFFUL;

    String lastPolishTime_;
    String lastSecondTime_;
    String lastPolishDate_;

    void findLandscapeRotation();
    void processPowerButton();
    void processTouch();
    void processEncoder();
    void updateStatusAndTimers();
    void updatePowerAndStandby();

    void noteActivity();
    bool overlayVisible() const;
    void cancelStandbyPrompt();
    void enterStandby();
    void wakeFromStandby();
    void shutdownDevice();

    void navigate(int8_t delta);
    void setScreen(Screen screen);
    void handleTap(int16_t x, int16_t y);
    void handleAppsTap(int16_t x, int16_t y);
    void handleMediaTap(int16_t x, int16_t y);
    void handleWindowsTap(int16_t x, int16_t y);
    void handleSettingsTap(int16_t x, int16_t y);
    void handleConnectionsTap(int16_t x, int16_t y);
    void handleHostOsPromptTap(int16_t x, int16_t y);

    void selectHostOS(HostOS os);
    void handlePairingConfirmationTap(int16_t x, int16_t y);
    void handlePowerMenuTap(int16_t x, int16_t y);
    void handleStandbyPromptTap(int16_t x, int16_t y);

    void draw();
    void drawStatusBar(const char* title);
    void drawCurrentStatusBar();
    void drawPageIndicator(uint16_t backgroundColor);
    void drawClockScreen();
    void drawClockValues(bool force);
    void drawClockCard(int16_t y, const String& value, uint16_t cardColor, uint16_t backgroundColor);
    void drawAppsScreen();
    void drawMediaScreen();
    void drawWindowsScreen();
    void drawSettingsScreen();
    void drawConnectionsScreen();
    void drawPairingConfirmation();
    void drawHostOSPrompt();

    void drawPowerMenu();
    void drawStandbyPrompt();
    void drawToast();

    void drawTile(
        int16_t x,
        int16_t y,
        int16_t width,
        int16_t height,
        Icon icon,
        const String& label,
        uint16_t fillColor,
        uint16_t borderColor
    );
    void drawIcon(Icon icon, int16_t centerX, int16_t centerY, uint16_t foreground, uint16_t background);
    void drawWifiStatusIcon(int16_t centerX, int16_t centerY, uint16_t iconColor);
    void drawBluetoothStatusIcon(int16_t centerX, int16_t centerY, uint16_t iconColor);
    void drawThickLine(
        int16_t x0,
        int16_t y0,
        int16_t x1,
        int16_t y1,
        uint16_t colorValue,
        uint8_t thickness = 3
    );
    void drawSettingRow(
        int16_t y,
        const String& label,
        const String& value,
        bool showMinusPlus,
        bool showToggle,
        bool toggleValue
    );
    void drawToggle(int16_t x, int16_t y, bool enabled);
    void drawBattery(int16_t x, int16_t y, int8_t level, bool charging);
    void showToast(const String& message, uint32_t durationMs = 1500);
    bool requireBluetooth();
    bool sendEncoderAction(bool increaseVolume);
    uint8_t standbyCountdownRemaining() const;

    static uint16_t color(uint8_t red, uint8_t green, uint8_t blue);
    static int8_t readBatteryLevel();
};
