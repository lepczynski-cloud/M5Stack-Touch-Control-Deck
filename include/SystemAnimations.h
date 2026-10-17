#pragma once

#include <Arduino.h>

// Animacje sa rysowane wyłącznie z prostych ksztaltow M5GFX. Dzięki temu
// projekt nie wymaga dodatkowych bitmap ani plikow zasobow.
class SystemAnimations {
public:
    static void startup();
    static void standby();
    static void wake();
    static void shutdown();

private:
    static uint16_t color(uint8_t red, uint8_t green, uint8_t blue);
    static void drawDeckMark(int16_t centerX, int16_t centerY, int16_t size, uint16_t accent, uint16_t muted);
    static void drawSpinner(int16_t centerX, int16_t centerY, int16_t radius, uint8_t phase, uint16_t accent);
};
