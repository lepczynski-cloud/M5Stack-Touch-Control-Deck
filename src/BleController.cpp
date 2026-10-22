#include "BleController.h"

#include <Esp.h>
#include <NimBLEDevice.h>

#include "UserConfig.h"

BleController::BleController()
    : keyboard_(
          UserDefaults::BLE_DEVICE_NAME,
          UserDefaults::BLE_MANUFACTURER,
          100
      ) {}

void BleController::begin(uint8_t initialBatteryLevel) {
    keyboard_.setDelay(12);
    keyboard_.setBatteryLevel(initialBatteryLevel);
    keyboard_.begin();
}

void BleController::loop(uint8_t batteryLevel) {
    const uint32_t now = millis();
    if (now - lastBatteryUpdateMillis_ >= 60000UL) {
        keyboard_.setBatteryLevel(batteryLevel);
        lastBatteryUpdateMillis_ = now;
    }
}

bool BleController::isConnected() const {
    return const_cast<BleKeyboard&>(keyboard_).isConnected();
}

bool BleController::openUrl(const String& url) {
    if (!ready() || url.isEmpty()) {
        return false;
    }

    // Win+R dziala na Windows 10 i 11. Adres jest wpisywany tak jak przez
    // zwykla klawiature, a system otwiera go w domyslnej przegladarce.
    chord({KEY_LEFT_GUI, static_cast<uint8_t>('r')});
    delay(UserDefaults::RUN_DIALOG_DELAY_MS);
    keyboard_.print(url);
    delay(UserDefaults::URL_SUBMIT_DELAY_MS);
    keyboard_.write(KEY_RETURN);
    return true;
}

bool BleController::mediaPrevious() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_PREVIOUS_TRACK);
    return true;
}

bool BleController::mediaPlayPause() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_PLAY_PAUSE);
    return true;
}

bool BleController::mediaNext() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_NEXT_TRACK);
    return true;
}

bool BleController::mediaMute() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_MUTE);
    return true;
}

bool BleController::mediaVolumeDown() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_VOLUME_DOWN);
    return true;
}

bool BleController::mediaVolumeUp() {
    if (!ready()) return false;
    keyboard_.write(KEY_MEDIA_VOLUME_UP);
    return true;
}

bool BleController::openTaskManager() {
    return chord({KEY_LEFT_CTRL, KEY_LEFT_SHIFT, KEY_ESC});
}

bool BleController::lockWindows() {
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('l')});
}

bool BleController::openSnippingTool() {
    // Win+Shift+S uruchamia narzedzie pozwalajace zaznaczyc wycinek ekranu.
    // Nie wykonuje automatycznie pelnego zrzutu ekranu.
    return chord({KEY_LEFT_GUI, KEY_LEFT_SHIFT, static_cast<uint8_t>('s')});
}

bool BleController::showDesktop() {
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('d')});
}

bool BleController::openFileExplorer() {
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('e')});
}

bool BleController::switchWindow() {
    return chord({KEY_LEFT_ALT, KEY_TAB}, 120);
}

void BleController::forgetBondAndRestart() {
    keyboard_.releaseAll();
    NimBLEDevice::deleteAllBonds();
    delay(600);
    ESP.restart();
}

bool BleController::ready() const {
    return isConnected();
}

bool BleController::chord(std::initializer_list<uint8_t> keys, uint32_t holdMs) {
    if (!ready()) {
        return false;
    }

    for (const uint8_t key : keys) {
        keyboard_.press(key);
    }
    delay(holdMs);
    keyboard_.releaseAll();
    return true;
}
