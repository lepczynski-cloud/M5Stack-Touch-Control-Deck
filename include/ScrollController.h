#pragma once

#include <Arduino.h>

class ScrollController {
public:
    bool begin();
    bool isAvailable() const;

    int16_t getEncoderValue();
    bool getButtonStatus();
    void setLEDColor(uint32_t colorValue);

private:
    bool available_ = false;
    int16_t lastEncoderValue_ = 0;
    bool lastButtonPressed_ = false;

    bool readRegister(uint8_t registerAddress, uint8_t* buffer, size_t length);
    bool writeRegister(uint8_t registerAddress, const uint8_t* buffer, size_t length);
};
