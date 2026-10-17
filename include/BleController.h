#pragma once

#include <Arduino.h>
#include <NimBleKeyboard.h>
#include <initializer_list>

class BleController {
public:
    BleController();

    void begin(uint8_t initialBatteryLevel);
    void loop(uint8_t batteryLevel);

    bool isConnected() const;
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

    bool ready() const;
    bool chord(std::initializer_list<uint8_t> keys, uint32_t holdMs = 70);
};
