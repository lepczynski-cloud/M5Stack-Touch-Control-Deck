#include "SystemAnimations.h"

#include <M5Unified.h>

namespace {
constexpr int16_t SCREEN_WIDTH = 320;
constexpr int16_t SCREEN_HEIGHT = 240;
constexpr uint16_t FRAME_DELAY_MS = 34;
}

void SystemAnimations::startup() {
    const uint16_t background = color(7, 12, 22);
    const uint16_t panel = color(18, 29, 47);
    const uint16_t accent = color(52, 166, 255);
    const uint16_t secondary = color(90, 102, 255);
    const uint16_t muted = color(82, 103, 132);

    M5.Display.fillScreen(background);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextWrap(false);

    for (uint8_t frame = 0; frame < 28; ++frame) {
        M5.Display.fillRect(0, 0, SCREEN_WIDTH, 174, background);

        const int16_t pulse = frame < 14 ? frame : 27 - frame;
        const int16_t markSize = 34 + pulse / 2;
        M5.Display.fillCircle(160, 82, 50, panel);
        M5.Display.drawCircle(160, 82, 50, color(29, 48, 75));
        M5.Display.drawCircle(160, 82, 43, color(23, 61, 94));
        drawSpinner(160, 82, 47, frame, accent);
        drawDeckMark(160, 82, markSize, accent, secondary);

        M5.Display.setTextFont(2);
        M5.Display.setTextColor(TFT_WHITE, background);
        M5.Display.drawString("M5 CONTROL DECK", 160, 151);
        M5.Display.setTextFont(1);
        M5.Display.setTextColor(color(151, 170, 197), background);
        M5.Display.drawString("Uruchamianie systemu", 160, 169);

        const int16_t progressWidth = static_cast<int16_t>((276L * (frame + 1)) / 28L);
        M5.Display.fillRoundRect(22, 202, 276, 7, 4, color(24, 38, 60));
        M5.Display.fillRoundRect(22, 202, progressWidth, 7, 4, frame < 18 ? accent : secondary);
        delay(FRAME_DELAY_MS);
    }

    delay(120);
}

void SystemAnimations::standby() {
    const uint16_t background = color(7, 12, 22);
    const uint16_t accent = color(67, 156, 255);
    const uint16_t muted = color(78, 96, 122);

    M5.Display.fillScreen(background);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextWrap(false);

    for (uint8_t frame = 0; frame < 15; ++frame) {
        M5.Display.fillRect(0, 34, SCREEN_WIDTH, 156, background);
        const int16_t radius = 44 - frame * 2;
        const int16_t safeRadius = radius < 16 ? 16 : radius;
        M5.Display.drawCircle(160, 91, safeRadius, frame < 9 ? accent : muted);
        M5.Display.fillCircle(160 + safeRadius / 3, 84, 5, accent);
        M5.Display.setTextFont(2);
        M5.Display.setTextColor(TFT_WHITE, background);
        M5.Display.drawString("Tryb czuwania", 160, 153);
        delay(28);
    }

    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(133, 151, 178), background);
    M5.Display.drawString("Dotknij ekranu, pokretla lub Power", 160, 179);
    delay(230);
}

void SystemAnimations::wake() {
    const uint16_t background = color(7, 12, 22);
    const uint16_t accent = color(52, 166, 255);
    const uint16_t secondary = color(92, 103, 255);

    M5.Display.fillScreen(background);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextWrap(false);

    for (uint8_t frame = 0; frame < 16; ++frame) {
        M5.Display.fillRect(0, 32, SCREEN_WIDTH, 150, background);
        const int16_t radius = 16 + frame * 2;
        M5.Display.drawCircle(160, 91, radius, frame < 9 ? secondary : accent);
        drawDeckMark(160, 91, 24 + frame, accent, secondary);
        M5.Display.setTextFont(2);
        M5.Display.setTextColor(TFT_WHITE, background);
        M5.Display.drawString("Witaj ponownie", 160, 157);
        delay(26);
    }
    delay(90);
}

void SystemAnimations::shutdown() {
    const uint16_t background = color(7, 12, 22);
    const uint16_t accent = color(255, 91, 111);
    const uint16_t muted = color(80, 92, 112);

    M5.Display.fillScreen(background);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextWrap(false);

    for (uint8_t frame = 0; frame < 18; ++frame) {
        M5.Display.fillRect(0, 30, SCREEN_WIDTH, 164, background);
        const int16_t radius = 42 - frame;
        M5.Display.drawCircle(160, 87, radius, frame < 12 ? accent : muted);
        M5.Display.drawLine(160, 56 + frame / 3, 160, 83, accent);
        M5.Display.fillCircle(160, 55 + frame / 3, 3, accent);

        M5.Display.setTextFont(2);
        M5.Display.setTextColor(TFT_WHITE, background);
        M5.Display.drawString("Wylaczanie", 160, 151);
        M5.Display.setTextFont(1);
        M5.Display.setTextColor(color(139, 155, 180), background);
        M5.Display.drawString("Zapisywanie ustawien", 160, 174);
        delay(32);
    }
    delay(180);
}

uint16_t SystemAnimations::color(uint8_t red, uint8_t green, uint8_t blue) {
    return static_cast<uint16_t>(((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3));
}

void SystemAnimations::drawDeckMark(
    int16_t centerX,
    int16_t centerY,
    int16_t size,
    uint16_t accent,
    uint16_t muted
) {
    const int16_t tile = size / 3;
    const int16_t gap = 3;
    const int16_t left = centerX - tile - gap / 2;
    const int16_t top = centerY - tile - gap / 2;

    M5.Display.fillRoundRect(left, top, tile, tile, 4, accent);
    M5.Display.fillRoundRect(left + tile + gap, top, tile, tile, 4, muted);
    M5.Display.fillRoundRect(left, top + tile + gap, tile, tile, 4, muted);
    M5.Display.fillRoundRect(left + tile + gap, top + tile + gap, tile, tile, 4, accent);
}

void SystemAnimations::drawSpinner(
    int16_t centerX,
    int16_t centerY,
    int16_t radius,
    uint8_t phase,
    uint16_t accent
) {
    static constexpr int8_t X[8] = {0, 7, 10, 7, 0, -7, -10, -7};
    static constexpr int8_t Y[8] = {-10, -7, 0, 7, 10, 7, 0, -7};

    for (uint8_t index = 0; index < 8; ++index) {
        const uint8_t distance = static_cast<uint8_t>((index + 8 - (phase % 8)) % 8);
        const int16_t dotRadius = distance == 0 ? 4 : (distance < 3 ? 3 : 2);
        const uint16_t dotColor = distance < 3 ? accent : color(39, 68, 102);
        M5.Display.fillCircle(
            centerX + static_cast<int16_t>(X[index]) * radius / 10,
            centerY + static_cast<int16_t>(Y[index]) * radius / 10,
            dotRadius,
            dotColor
        );
    }
}
