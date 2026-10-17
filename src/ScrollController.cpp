#include "ScrollController.h"

#include <M5Unified.h>

#include "UserConfig.h"

namespace {
constexpr uint8_t ENCODER_REGISTER = 0x10;
constexpr uint8_t BUTTON_REGISTER = 0x20;
constexpr uint8_t RGB_LED_REGISTER = 0x30;
}

bool ScrollController::begin() {
    // CoreS3 SE ma osobna magistrale I2C dla Port A. M5Unified przypisuje
    // jej GPIO 2 jako SDA i GPIO 1 jako SCL, dlatego nie uruchamiamy tu
    // ponownie globalnego Wire uzywanego przez inne elementy plytki.
    if (!M5.Ex_I2C.begin(
            I2C_NUM_0,
            UserDefaults::SCROLL_SDA_PIN,
            UserDefaults::SCROLL_SCL_PIN
        )) {
        available_ = false;
        return false;
    }

    available_ = M5.Ex_I2C.scanID(
        UserDefaults::SCROLL_I2C_ADDRESS,
        UserDefaults::SCROLL_I2C_SPEED
    );

    if (available_) {
        lastEncoderValue_ = getEncoderValue();
    }
    return available_;
}

bool ScrollController::isAvailable() const {
    return available_;
}

int16_t ScrollController::getEncoderValue() {
    uint8_t buffer[2] = {};
    if (!readRegister(ENCODER_REGISTER, buffer, sizeof(buffer))) {
        return lastEncoderValue_;
    }

    lastEncoderValue_ = static_cast<int16_t>(
        static_cast<uint16_t>(buffer[0]) |
        (static_cast<uint16_t>(buffer[1]) << 8)
    );
    return lastEncoderValue_;
}

bool ScrollController::getButtonStatus() {
    uint8_t value = 0xFF;
    if (!readRegister(BUTTON_REGISTER, &value, 1)) {
        // Przy pojedynczym bledzie I2C zachowujemy poprzedni stan. Dzieki temu
        // chwilowy brak odpowiedzi nie jest traktowany jak puszczenie przycisku.
        return lastButtonPressed_;
    }

    // Firmware Scroll Unit zwraca 0x00 podczas nacisniecia przycisku.
    lastButtonPressed_ = value == 0x00;
    return lastButtonPressed_;
}

void ScrollController::setLEDColor(uint32_t colorValue) {
    const uint8_t buffer[4] = {
        0,
        static_cast<uint8_t>((colorValue >> 16) & 0xFF),
        static_cast<uint8_t>((colorValue >> 8) & 0xFF),
        static_cast<uint8_t>(colorValue & 0xFF),
    };
    writeRegister(RGB_LED_REGISTER, buffer, sizeof(buffer));
}

bool ScrollController::readRegister(uint8_t registerAddress, uint8_t* buffer, size_t length) {
    if (!available_) {
        return false;
    }

    const bool result = M5.Ex_I2C.readRegister(
        UserDefaults::SCROLL_I2C_ADDRESS,
        registerAddress,
        buffer,
        length,
        UserDefaults::SCROLL_I2C_SPEED
    );
    return result;
}

bool ScrollController::writeRegister(uint8_t registerAddress, const uint8_t* buffer, size_t length) {
    if (!available_) {
        return false;
    }

    const bool result = M5.Ex_I2C.writeRegister(
        UserDefaults::SCROLL_I2C_ADDRESS,
        registerAddress,
        buffer,
        length,
        UserDefaults::SCROLL_I2C_SPEED
    );
    return result;
}
