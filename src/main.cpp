#include <Arduino.h>
#include <M5Unified.h>

#include "BleController.h"
#include "ConfigManager.h"
#include "NetworkPortal.h"
#include "ScrollController.h"
#include "SystemAnimations.h"
#include "TimeManager.h"
#include "UserConfig.h"
#include "UserInterface.h"

ConfigManager configManager;
TimeManager timeManager(configManager);
BleController bleController;
NetworkPortal networkPortal(configManager, timeManager, bleController);
ScrollController scrollUnit;
UserInterface userInterface(
    configManager,
    timeManager,
    networkPortal,
    bleController,
    scrollUnit
);

namespace {
uint8_t initialBatteryLevel() {
    const int8_t value = M5.Power.getBatteryLevel();
    return value >= 0 && value <= 100 ? static_cast<uint8_t>(value) : 100;
}

void setLandscapeRotation() {
    for (uint8_t rotation = 0; rotation < 4; ++rotation) {
        M5.Display.setRotation(rotation);
        if (M5.Display.width() == 320 && M5.Display.height() == 240) {
            return;
        }
    }
    M5.Display.setRotation(1);
}

uint16_t rgb565(uint8_t red, uint8_t green, uint8_t blue) {
    return static_cast<uint16_t>(((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3));
}

void showBootStep(const String& text, int16_t y, bool done = false) {
    const uint16_t background = rgb565(9, 14, 24);
    const uint16_t foreground = done
        ? rgb565(0, 205, 135)
        : rgb565(190, 205, 224);
    M5.Display.fillRect(18, y - 9, 284, 20, background);
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(foreground, background);
    M5.Display.drawString((done ? "[OK] " : "[...] ") + text, 20, y);
}
}

void setup() {
    Serial.begin(115200);
    delay(150);

    auto m5Config = M5.config();
    m5Config.clear_display = true;
    m5Config.output_power = true;
    M5.begin(m5Config);

    setLandscapeRotation();
    M5.Display.setBrightness(UserDefaults::DISPLAY_BRIGHTNESS);
    SystemAnimations::startup();
    M5.Display.fillScreen(rgb565(9, 14, 24));
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("M5Stack Touch Control Deck", 160, 24);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(rgb565(142, 162, 189));
    M5.Display.drawString("Uruchamianie firmware...", 160, 44);

    showBootStep("Konfiguracja", 72);
    configManager.begin();
    M5.Display.setBrightness(configManager.data().brightness);
    showBootStep("Konfiguracja", 72, true);

    showBootStep("Zegar i RTC", 96);
    timeManager.begin();
    showBootStep("Zegar i RTC", 96, true);

    showBootStep("Scroll Unit", 120);
    const bool scrollAvailable = scrollUnit.begin();
    showBootStep(scrollAvailable ? "Scroll Unit" : "Scroll Unit: brak", 120, scrollAvailable);

    showBootStep("Wi-Fi i portal", 144);
    networkPortal.begin();
    showBootStep("Wi-Fi i portal", 144, true);

    showBootStep("Klawiatura Bluetooth", 168);
    bleController.setHostOS(configManager.data().hostOs);
    bleController.begin(initialBatteryLevel());
    showBootStep("Klawiatura Bluetooth", 168, true);

    userInterface.begin(scrollAvailable);

    Serial.println();
    Serial.println(UserDefaults::PROJECT_NAME);
    Serial.printf("Firmware: %s\n", UserDefaults::FIRMWARE_VERSION);
    Serial.printf("Scroll Unit: %s\n", scrollAvailable ? "OK" : "brak polaczenia");
}

void loop() {
    M5.update();
    networkPortal.loop();
    timeManager.loop(networkPortal.isWifiConnected());
    bleController.loop(initialBatteryLevel());
    userInterface.loop();
    delay(8);
}
