#include "UserInterface.h"

#include <algorithm>
#include <cmath>

#include "SystemAnimations.h"
#include "UserConfig.h"

namespace {
constexpr int16_t DISPLAY_WIDTH = 320;
constexpr int16_t DISPLAY_HEIGHT = 240;
constexpr int16_t STATUS_BAR_HEIGHT = 25;
constexpr int16_t PAGE_INDICATOR_Y = 229;

// Gest jest uznawany za przesuniecie juz przy 26 pikselach. Klikniecie wymaga
// bardzo malego ruchu, co zapobiega uruchamianiu kafelka podczas przewijania.
constexpr int16_t SWIPE_THRESHOLD = 26;
constexpr int16_t SWIPE_DOMINANCE_MARGIN = 8;
constexpr int16_t TAP_MAX_MOVEMENT = 8;
constexpr uint32_t TAP_MAX_DURATION_MS = 700;

constexpr uint32_t ENCODER_POLL_INTERVAL_MS = 18;
constexpr uint32_t ENCODER_BUTTON_MIN_PRESS_MS = 35;
constexpr uint32_t ENCODER_BUTTON_MAX_PRESS_MS = 1200;
constexpr uint32_t ENCODER_CLICK_COOLDOWN_MS = 300;

constexpr int16_t POWER_BUTTONS_Y = 148;
constexpr int16_t POWER_BUTTONS_HEIGHT = 42;
constexpr int16_t POWER_BUTTON_1_X = 22;
constexpr int16_t POWER_BUTTON_2_X = 117;
constexpr int16_t POWER_BUTTON_3_X = 212;
constexpr int16_t POWER_BUTTON_WIDTH = 86;

constexpr int16_t STANDBY_BUTTONS_Y = 150;
constexpr int16_t STANDBY_BUTTONS_HEIGHT = 40;
constexpr int16_t STANDBY_CANCEL_X = 36;
constexpr int16_t STANDBY_NOW_X = 170;
constexpr int16_t STANDBY_BUTTON_WIDTH = 114;

constexpr int16_t TILE_X = 5;
constexpr int16_t TILE_Y = 30;
constexpr int16_t TILE_WIDTH = 100;
constexpr int16_t TILE_HEIGHT = 88;
constexpr int16_t TILE_X_STEP = 105;
constexpr int16_t TILE_Y_STEP = 94;

int tileIndexAt(int16_t x, int16_t y) {
    for (int index = 0; index < 6; ++index) {
        const int column = index % 3;
        const int row = index / 3;
        const int16_t left = TILE_X + column * TILE_X_STEP;
        const int16_t top = TILE_Y + row * TILE_Y_STEP;
        if (x >= left && x < left + TILE_WIDTH && y >= top && y < top + TILE_HEIGHT) {
            return index;
        }
    }
    return -1;
}
}

UserInterface::UserInterface(
    ConfigManager& configManager,
    TimeManager& timeManager,
    NetworkPortal& networkPortal,
    BleController& bleController,
    ScrollController& scrollUnit
)
    : configManager_(configManager),
      timeManager_(timeManager),
      networkPortal_(networkPortal),
      bleController_(bleController),
      scrollUnit_(scrollUnit),
      clockCardCanvas_(&M5.Display) {}

void UserInterface::begin(bool scrollAvailable) {
    scrollAvailable_ = scrollAvailable;
    findLandscapeRotation();

    M5.Display.setBrightness(configManager_.data().brightness);
    M5.Display.setTextWrap(false);
    M5.Display.fillScreen(color(9, 14, 24));

    // Bufor 280 x 58 pikseli pozwala przygotowac caly kafel zegara w pamieci
    // i wyslac go na ekran jednym ruchem. Dzieki temu cyfry nie migaja.
    clockCardCanvas_.setColorDepth(16);
    clockCardCanvasReady_ = clockCardCanvas_.createSprite(280, 58) != nullptr;

    if (scrollAvailable_) {
        lastEncoderValue_ = scrollUnit_.getEncoderValue();
        scrollUnit_.setLEDColor(0x258DFF);
    }

    lastBleConnected_ = bleController_.isConnected();
    lastWifiConnected_ = networkPortal_.isWifiConnected();
    lastApActive_ = networkPortal_.isApActive();
    lastBatteryLevel_ = readBatteryLevel();
    lastCharging_ = M5.Power.isCharging();
    lastStandbyBleConnected_ = lastBleConnected_;
    lastInteractionMillis_ = millis();
    dirty_ = true;
    statusBarDirty_ = true;
    clockValuesDirty_ = true;
    draw();
}

void UserInterface::loop() {
    processPowerButton();
    processTouch();
    processEncoder();
    updateStatusAndTimers();
    updatePowerAndStandby();

    if (standby_) {
        return;
    }

    const bool toastActive = !toastMessage_.isEmpty() && static_cast<int32_t>(millis() - toastUntilMillis_) < 0;

    if (dirty_) {
        draw();
    } else {
        if (statusBarDirty_ && !overlayVisible()) {
            drawCurrentStatusBar();
            statusBarDirty_ = false;
        }

        if (screen_ == Screen::Clock && clockValuesDirty_ && !toastActive && !overlayVisible()) {
            drawClockValues(false);
        }
    }

    if (!toastMessage_.isEmpty() && !toastActive) {
        toastMessage_ = "";
        dirty_ = true;
    }
}

void UserInterface::requestRedraw() {
    dirty_ = true;
}

void UserInterface::findLandscapeRotation() {
    // Rozne wersje bibliotek moga startowac z inna orientacja. Wybieramy
    // automatycznie ustawienie, ktore daje natywna rozdzielczosc 320 x 240.
    for (uint8_t rotation = 0; rotation < 4; ++rotation) {
        M5.Display.setRotation(rotation);
        if (M5.Display.width() == DISPLAY_WIDTH && M5.Display.height() == DISPLAY_HEIGHT) {
            return;
        }
    }
    M5.Display.setRotation(1);
}

void UserInterface::processPowerButton() {
    // Dlugie przytrzymanie od razu rozpoczyna bezpieczne wylaczanie.
    if (M5.BtnPWR.wasHold()) {
        shutdownDevice();
        return;
    }

    if (!M5.BtnPWR.wasClicked()) {
        return;
    }

    if (standby_) {
        wakeFromStandby();
        return;
    }

    noteActivity();
    standbyPromptVisible_ = false;
    pairingConfirmationVisible_ = false;
    if (hostOsPromptVisible_ || macSpotlightPromptVisible_) return; // Pytania maja pierwszenstwo.
    powerMenuVisible_ = !powerMenuVisible_;
    dirty_ = true;
}

void UserInterface::processTouch() {
    const auto detail = M5.Touch.getDetail();
    const bool pressed = detail.isPressed();

    // Pierwsze dotkniecie po wygaszeniu jedynie wybudza ekran. Nie wykonuje
    // akcji znajdujacej sie pod palcem.
    if (standby_) {
        if (pressed && !touchDown_) {
            touchDown_ = true;
            suppressTouchUntilRelease_ = true;
            wakeFromStandby();
        } else if (!pressed) {
            touchDown_ = false;
        }
        return;
    }

    if (suppressTouchUntilRelease_) {
        if (!pressed) {
            suppressTouchUntilRelease_ = false;
            touchDown_ = false;
        }
        return;
    }

    if (pressed) {
        if (!touchDown_) {
            touchDown_ = true;
            touchStartX_ = detail.x;
            touchStartY_ = detail.y;
            touchLastX_ = detail.x;
            touchLastY_ = detail.y;
            touchMaxDistance_ = 0;
            touchStartMillis_ = millis();
            noteActivity();
        }

        touchLastX_ = detail.x;
        touchLastY_ = detail.y;
        const int16_t distanceX = std::abs(touchLastX_ - touchStartX_);
        const int16_t distanceY = std::abs(touchLastY_ - touchStartY_);
        touchMaxDistance_ = std::max(touchMaxDistance_, std::max(distanceX, distanceY));
        return;
    }

    if (!touchDown_) {
        return;
    }

    touchDown_ = false;
    const int16_t deltaX = touchLastX_ - touchStartX_;
    const int16_t deltaY = touchLastY_ - touchStartY_;
    const int16_t absoluteX = std::abs(deltaX);
    const int16_t absoluteY = std::abs(deltaY);
    const uint32_t touchDuration = millis() - touchStartMillis_;
    const bool cleanTap = touchMaxDistance_ <= TAP_MAX_MOVEMENT && touchDuration <= TAP_MAX_DURATION_MS;

    if (powerMenuVisible_) {
        if (cleanTap) {
            handlePowerMenuTap(touchLastX_, touchLastY_);
        }
        return;
    }

    if (standbyPromptVisible_) {
        if (cleanTap) {
            handleStandbyPromptTap(touchLastX_, touchLastY_);
        } else {
            cancelStandbyPrompt();
        }
        return;
    }

    if (hostOsPromptVisible_) {
        if (cleanTap) handleHostOsPromptTap(touchLastX_, touchLastY_);
        return;
    }

    if (pairingConfirmationVisible_) {
        if (cleanTap) {
            handlePairingConfirmationTap(touchLastX_, touchLastY_);
        }
        return;
    }

    if (macSpotlightPromptVisible_) {
        if (cleanTap) handleMacSpotlightPromptTap(touchLastX_, touchLastY_);
        return;
    }

    if (
        absoluteY >= SWIPE_THRESHOLD &&
        absoluteY >= absoluteX + SWIPE_DOMINANCE_MARGIN
    ) {
        navigate(deltaY < 0 ? 1 : -1);
        return;
    }

    // Ruch pomiedzy progiem klikniecia i progiem gestu jest ignorowany.
    // To celowe zabezpieczenie przed przypadkowym uruchamianiem kafelkow.
    if (cleanTap) {
        handleTap(touchLastX_, touchLastY_);
    }
}

void UserInterface::processEncoder() {
    if (hostOsPromptVisible_) return; // Nie wysylamy klawiszy przed wyborem systemu.
    if (!scrollAvailable_) {
        return;
    }

    const uint32_t now = millis();
    if (now - lastEncoderPollMillis_ < ENCODER_POLL_INTERVAL_MS) {
        return;
    }
    lastEncoderPollMillis_ = now;

    const int16_t encoderValue = scrollUnit_.getEncoderValue();
    const bool encoderMoved = encoderValue != lastEncoderValue_;
    int32_t difference = 0;
    if (encoderMoved) {
        difference = static_cast<int16_t>(encoderValue - lastEncoderValue_);
        lastEncoderValue_ = encoderValue;
        if (UserDefaults::SCROLL_REVERSE_DIRECTION) {
            difference = -difference;
        }
    }

    const bool buttonPressed = scrollUnit_.getButtonStatus();

    if (standby_) {
        if (encoderMoved || (buttonPressed && !encoderButtonDown_)) {
            suppressEncoderClickUntilRelease_ = buttonPressed;
            encoderButtonDown_ = buttonPressed;
            wakeFromStandby();
        }
        return;
    }

    if (encoderMoved) {
        noteActivity();
        if (standbyPromptVisible_) {
            cancelStandbyPrompt();
            return;
        }

        if (!overlayVisible()) {
            // Odczyt jest wartoscia skumulowana, dlatego szybki obrot nie gubi
            // krokow pomiedzy probkami. Ograniczenie chroni przed wyslaniem
            // bardzo dlugiej serii po przypadkowym skoku wartosci.
            const int32_t steps = std::min<int32_t>(6, std::abs(difference));
            for (int32_t step = 0; step < steps; ++step) {
                sendEncoderAction(difference > 0);
                delay(6);
            }
        }
    }

    if (buttonPressed && !encoderButtonDown_) {
        encoderButtonDown_ = true;
        encoderButtonDownMillis_ = now;
        noteActivity();
        if (standbyPromptVisible_) {
            cancelStandbyPrompt();
            suppressEncoderClickUntilRelease_ = true;
        }
        return;
    }

    if (!buttonPressed && encoderButtonDown_) {
        encoderButtonDown_ = false;
        const uint32_t pressDuration = now - encoderButtonDownMillis_;

        if (suppressEncoderClickUntilRelease_) {
            suppressEncoderClickUntilRelease_ = false;
            return;
        }

        if (
            !overlayVisible() &&
            pressDuration >= ENCODER_BUTTON_MIN_PRESS_MS &&
            pressDuration < ENCODER_BUTTON_MAX_PRESS_MS &&
            now - lastEncoderClickMillis_ >= ENCODER_CLICK_COOLDOWN_MS
        ) {
            lastEncoderClickMillis_ = now;
            if (!bleController_.mediaPlayPause() && now - lastEncoderDisconnectedToastMillis_ >= 1500) {
                showToast("Najpierw polacz Bluetooth");
                lastEncoderDisconnectedToastMillis_ = now;
            }
        }
    }
}

void UserInterface::updateStatusAndTimers() {
    const uint32_t now = millis();

    if (now - lastStatusRefreshMillis_ >= 500) {
        lastStatusRefreshMillis_ = now;

        const bool bleConnected = bleController_.isConnected();
        const bool wifiConnected = networkPortal_.isWifiConnected();
        const bool apActive = networkPortal_.isApActive();
        const bool charging = M5.Power.isCharging();

        int8_t batteryLevel = lastBatteryLevel_;
        if (now - lastBatteryRefreshMillis_ >= 10000UL) {
            lastBatteryRefreshMillis_ = now;
            batteryLevel = readBatteryLevel();
        }

        // NimBLE udostepnia zapisany bond, lecz nie nazwe systemu komputera.
        // Dla nowego powiazania pokazujemy wybor systemu tylko raz, nie po
        // kazdym zwyklym rozlaczeniu i ponownym polaczeniu.
        if (bleConnected) {
            const String peerId = bleController_.bondedPeerIdentity();
            if (!peerId.isEmpty()) {
                if (peerId != configManager_.data().pairedHostId) {
                    if (!hostOsPromptVisible_ || pendingHostPeerId_ != peerId) {
                        hostOsPromptVisible_ = true;
                        pendingHostPeerId_ = peerId;
                        powerMenuVisible_ = false;
                        pairingConfirmationVisible_ = false;
                        standbyPromptVisible_ = false;
                        if (standby_) wakeFromStandby();
                        dirty_ = true;
                    }
                } else if (hostOsPromptVisible_) {
                    // Host zostal zatwierdzony z lokalnego portalu WWW.
                    hostOsPromptVisible_ = false;
                    pendingHostPeerId_ = "";
                    bleController_.setHostOS(configManager_.data().hostOs);
                    dirty_ = true;
                }
            }
        }

        const bool bleChanged = bleConnected != lastBleConnected_;
        const bool wifiChanged = wifiConnected != lastWifiConnected_;
        const bool apChanged = apActive != lastApActive_;
        const bool powerChanged = charging != lastCharging_ || batteryLevel != lastBatteryLevel_;

        lastBleConnected_ = bleConnected;
        lastWifiConnected_ = wifiConnected;
        lastApActive_ = apActive;
        lastCharging_ = charging;
        lastBatteryLevel_ = batteryLevel;

        // Zmiana Wi-Fi zmienia kolor calego ekranu zegara. Pozostale dane
        // odswiezamy tylko tam, gdzie sa widoczne, aby nie powodowac migania.
        if (
            (wifiChanged && screen_ == Screen::Clock) ||
            ((wifiChanged || apChanged) && (screen_ == Screen::Settings || screen_ == Screen::Connections)) ||
            (bleChanged && screen_ == Screen::Connections)
        ) {
            dirty_ = true;
        } else if (bleChanged || wifiChanged || powerChanged) {
            statusBarDirty_ = true;
        }

        if (scrollAvailable_) {
            uint32_t ledColor = 0x8E44AD;
            if (apActive) {
                ledColor = 0xFF7A00;
            } else if (bleConnected) {
                ledColor = 0x00C878;
            } else if (wifiConnected) {
                ledColor = 0x258DFF;
            }
            if (ledColor != lastScrollLedColor_) {
                scrollUnit_.setLEDColor(ledColor);
                lastScrollLedColor_ = ledColor;
            }
        }
    }

    if (screen_ == Screen::Clock && now - lastClockRefreshMillis_ >= 1000) {
        lastClockRefreshMillis_ = now;
        clockValuesDirty_ = true;
    }
}

void UserInterface::updatePowerAndStandby() {
    const uint32_t now = millis();
    const bool bleConnected = bleController_.isConnected();

    if (bleConnected != lastStandbyBleConnected_) {
        lastStandbyBleConnected_ = bleConnected;
        lastInteractionMillis_ = now;

        if (bleConnected) {
            if (standbyPromptVisible_) {
                cancelStandbyPrompt();
            }
            if (standby_) {
                wakeFromStandby();
            }
        }
    }

    // Aktywne polaczenie Bluetooth oznacza, ze panel jest uzywany razem z
    // komputerem. W takim stanie ekran nie przechodzi automatycznie w czuwanie.
    if (bleConnected) {
        lastInteractionMillis_ = now;
        return;
    }

    if (standby_) {
        return;
    }

    const auto& config = configManager_.data();
    if (!config.standbyEnabled) {
        if (standbyPromptVisible_) {
            cancelStandbyPrompt();
        }
        return;
    }

    if (powerMenuVisible_ || pairingConfirmationVisible_) {
        return;
    }

    if (standbyPromptVisible_) {
        const uint8_t remaining = standbyCountdownRemaining();
        if (remaining == 0) {
            enterStandby();
            return;
        }
        if (remaining != lastStandbyCountdownValue_) {
            lastStandbyCountdownValue_ = remaining;
            dirty_ = true;
        }
        return;
    }

    const uint32_t timeoutMs = static_cast<uint32_t>(config.standbyTimeoutMinutes) * 60UL * 1000UL;
    if (now - lastInteractionMillis_ >= timeoutMs) {
        standbyPromptVisible_ = true;
        standbyPromptStartedMillis_ = now;
        lastStandbyCountdownValue_ = UserDefaults::STANDBY_COUNTDOWN_SECONDS;
        toastMessage_ = "";
        dirty_ = true;
    }
}

void UserInterface::noteActivity() {
    lastInteractionMillis_ = millis();
}

bool UserInterface::overlayVisible() const {
    return hostOsPromptVisible_ || pairingConfirmationVisible_ || macSpotlightPromptVisible_ || powerMenuVisible_ || standbyPromptVisible_;
}

void UserInterface::cancelStandbyPrompt() {
    if (!standbyPromptVisible_) {
        return;
    }
    standbyPromptVisible_ = false;
    standbyPromptStartedMillis_ = 0;
    lastStandbyCountdownValue_ = 0xFF;
    noteActivity();
    dirty_ = true;
}

void UserInterface::enterStandby() {
    // Automatyczne czuwanie jest uruchamiane tylko bez polaczenia Bluetooth,
    // ale uzytkownik moze wybrac je recznie takze podczas polaczenia.
    powerMenuVisible_ = false;
    pairingConfirmationVisible_ = false;
    standbyPromptVisible_ = false;
    toastMessage_ = "";
    SystemAnimations::standby();
    M5.Display.setBrightness(0);
    M5.Display.sleep();
    standby_ = true;
    touchDown_ = false;
    lastStandbyCountdownValue_ = 0xFF;
}

void UserInterface::wakeFromStandby() {
    if (!standby_) {
        return;
    }

    M5.Display.wakeup();
    M5.Display.setBrightness(configManager_.data().brightness);
    delay(25);
    SystemAnimations::wake();

    standby_ = false;
    noteActivity();
    dirty_ = true;
    statusBarDirty_ = true;
    clockValuesDirty_ = true;
    lastClockRefreshMillis_ = 0;
}

void UserInterface::shutdownDevice() {
    powerMenuVisible_ = false;
    pairingConfirmationVisible_ = false;
    standbyPromptVisible_ = false;

    if (standby_) {
        M5.Display.wakeup();
        M5.Display.setBrightness(configManager_.data().brightness);
        delay(25);
        standby_ = false;
    }

    configManager_.save();
    SystemAnimations::shutdown();
    M5.Display.setBrightness(0);
    M5.Display.sleep();
    delay(80);

    // CoreS3 SE korzysta z ukladu zasilania AXP2101 obslugiwanego przez
    // M5Unified. Po odcieciu zasilania kod nie powinien juz wrocic z tej funkcji.
    M5.Power.powerOff();
    while (true) {
        delay(1000);
    }
}

uint8_t UserInterface::standbyCountdownRemaining() const {
    if (!standbyPromptVisible_) {
        return 0;
    }

    const uint32_t elapsedSeconds = (millis() - standbyPromptStartedMillis_) / 1000UL;
    if (elapsedSeconds >= UserDefaults::STANDBY_COUNTDOWN_SECONDS) {
        return 0;
    }
    return static_cast<uint8_t>(UserDefaults::STANDBY_COUNTDOWN_SECONDS - elapsedSeconds);
}

void UserInterface::navigate(int8_t delta) {
    const int count = static_cast<int>(Screen::Count);
    int index = static_cast<int>(screen_);
    index = (index + delta + count) % count;
    setScreen(static_cast<Screen>(index));
}

void UserInterface::setScreen(Screen screen) {
    if (screen_ == screen) {
        return;
    }
    screen_ = screen;
    dirty_ = true;
    if (screen_ == Screen::Clock) {
        clockValuesDirty_ = true;
    }
}

void UserInterface::handleTap(int16_t x, int16_t y) {
    if (y < STATUS_BAR_HEIGHT) {
        return;
    }

    switch (screen_) {
        case Screen::Apps: handleAppsTap(x, y); break;
        case Screen::Media: handleMediaTap(x, y); break;
        case Screen::Windows: handleWindowsTap(x, y); break;
        case Screen::Settings: handleSettingsTap(x, y); break;
        case Screen::Connections: handleConnectionsTap(x, y); break;
        default: break;
    }
}

void UserInterface::handleAppsTap(int16_t x, int16_t y) {
    const int index = tileIndexAt(x, y);
    if (index < 0 || index >= static_cast<int>(configManager_.data().apps.size())) {
        return;
    }

    if (!requireBluetooth()) {
        return;
    }

    const String url = configManager_.data().apps[index].url;
    if (configManager_.data().hostOs == HostOS::MacOS &&
        !configManager_.data().macSpotlightVerified) {
        if (!BleController::isSafeBrowserUrl(url)) {
            showToast("Niepoprawny adres URL");
            return;
        }
        requestMacSpotlightCheck(url);
        return;
    }
    if (bleController_.openUrl(url)) {
        showToast("Otwieram: " + configManager_.data().apps[index].label);
    }
}

void UserInterface::handleMediaTap(int16_t x, int16_t y) {
    const int index = tileIndexAt(x, y);
    if (index < 0 || !requireBluetooth()) {
        return;
    }

    bool sent = false;
    switch (index) {
        case 0: sent = bleController_.mediaPrevious(); break;
        case 1: sent = bleController_.mediaPlayPause(); break;
        case 2: sent = bleController_.mediaNext(); break;
        case 3: sent = bleController_.mediaMute(); break;
        case 4: sent = bleController_.mediaVolumeDown(); break;
        case 5: sent = bleController_.mediaVolumeUp(); break;
        default: break;
    }

    if (sent) {
        showToast("Polecenie wyslane", 900);
    }
}

void UserInterface::handleWindowsTap(int16_t x, int16_t y) {
    const int index = tileIndexAt(x, y);
    if (index < 0 || !requireBluetooth()) {
        return;
    }

    if (configManager_.data().hostOs == HostOS::MacOS &&
        !configManager_.data().macSpotlightVerified &&
        (index == 0 || index == 4)) {
        requestMacSpotlightCheck(index == 0 ? String("Activity Monitor") : String("Finder"));
        return;
    }

    bool sent = false;
    switch (index) {
        case 0: sent = bleController_.openTaskManager(); break;
        case 1: sent = bleController_.lockWindows(); break;
        case 2: sent = bleController_.openSnippingTool(); break;
        case 3: sent = bleController_.showDesktop(); break;
        case 4: sent = bleController_.openFileExplorer(); break;
        case 5: sent = bleController_.switchWindow(); break;
        default: break;
    }

    if (sent) {
        showToast("Skrot wyslany", 1000);
    }
}

void UserInterface::handleSettingsTap(int16_t x, int16_t y) {
    auto& config = configManager_.data();

    if (y >= 32 && y < 73) {
        if (x >= 205 && x < 258) {
            config.brightness = static_cast<uint8_t>(std::max(20, static_cast<int>(config.brightness) - 20));
        } else if (x >= 262) {
            config.brightness = static_cast<uint8_t>(std::min(255, static_cast<int>(config.brightness) + 20));
        } else {
            return;
        }
        M5.Display.setBrightness(config.brightness);
        configManager_.save();
        dirty_ = true;
        return;
    }

    if (y >= 76 && y < 117) {
        networkPortal_.setWifiEnabled(!config.wifiEnabled);
        showToast(config.wifiEnabled ? "Wi-Fi wlaczone" : "Wi-Fi wylaczone");
        dirty_ = true;
        return;
    }

    if (y >= 120 && y < 161) {
        networkPortal_.setApEnabled(!networkPortal_.isApActive());
        showToast(networkPortal_.isApActive() ? "AP wlaczony" : "AP wylaczony");
        dirty_ = true;
        return;
    }

    if (y >= 164 && y < 205) {
        int timezoneIndex = config.secondTimezoneIndex;
        if (x >= 205 && x < 258) {
            --timezoneIndex;
        } else if (x >= 262) {
            ++timezoneIndex;
        } else {
            return;
        }
        if (timezoneIndex < 0) timezoneIndex = TimeManager::TIMEZONE_COUNT - 1;
        if (timezoneIndex >= static_cast<int>(TimeManager::TIMEZONE_COUNT)) timezoneIndex = 0;
        config.secondTimezoneIndex = static_cast<uint8_t>(timezoneIndex);
        configManager_.save();
        lastSecondTime_ = "";
        dirty_ = true;
    }
}

void UserInterface::selectHostOS(HostOS os) {
    auto& config = configManager_.data();
    // Powrot do macOS lub zmiana komputera wymaga ponownego testu fokusu.
    if (os == HostOS::MacOS || config.hostOs != os) config.macSpotlightVerified = false;
    config.hostOs = os;
    // Osobny wybór podczas parowania oznacza zgode na ten konkretny host.
    const String peer = !pendingHostPeerId_.isEmpty()
        ? pendingHostPeerId_ : bleController_.bondedPeerIdentity();
    if (!peer.isEmpty()) config.pairedHostId = peer;
    configManager_.save();
    bleController_.setHostOS(os);
    hostOsPromptVisible_ = false;
    pendingHostPeerId_ = "";
    showToast(String("System: ") + hostOsName(os), 1600);
    dirty_ = true;
}

bool UserInterface::requestMacSpotlightCheck(const String& query) {
    if (!bleController_.isConnected() || query.isEmpty()) return false;
    if (!bleController_.openMacLauncherOnly()) return false;
    pendingMacQuery_ = query;
    macSpotlightPromptVisible_ = true;
    powerMenuVisible_ = false;
    standbyPromptVisible_ = false;
    pairingConfirmationVisible_ = false;
    dirty_ = true;
    return true;
}

void UserInterface::handleMacSpotlightPromptTap(int16_t x, int16_t y) {
    if (y < 148 || y > 203) return;
    if (x >= 20 && x <= 149) {
        // NIE: nie wpisujemy niczego do aktywnej aplikacji komputera.
        macSpotlightPromptVisible_ = false;
        pendingMacQuery_ = "";
        showToast("Sprawdz skrot Spotlight na Macu", 2300);
        return;
    }
    if (x >= 170 && x <= 299) {
        // TAK: dopiero po wzrokowym potwierdzeniu wysylamy znaki.
        const String query = pendingMacQuery_;
        pendingMacQuery_ = "";
        macSpotlightPromptVisible_ = false;
        if (bleController_.finishMacLauncherQuery(query)) {
            configManager_.data().macSpotlightVerified = true;
            configManager_.save();
            showToast("Spotlight potwierdzony");
        } else {
            showToast("Bluetooth rozlaczony");
        }
    }
}

void UserInterface::handleHostOsPromptTap(int16_t x, int16_t y) {
    if (y < 108 || y > 169) return;
    if (x >= 10 && x < 107) selectHostOS(HostOS::Windows);
    else if (x >= 111 && x < 210) selectHostOS(HostOS::MacOS);
    else if (x >= 214 && x < 310) selectHostOS(HostOS::LinuxGnome);
}

void UserInterface::handleConnectionsTap(int16_t x, int16_t y) {
    // Zmiana systemu bez restartu i bez koniecznosci ponownego parowania.
    if (y >= 99 && y <= 138) {
        if (x >= 12 && x < 111) selectHostOS(HostOS::Windows);
        else if (x >= 113 && x < 211) selectHostOS(HostOS::MacOS);
        else if (x >= 213 && x <= 309) selectHostOS(HostOS::LinuxGnome);
        return;
    }
    if (x >= 18 && x <= 302 && y >= 151 && y <= 186) {
        pairingConfirmationVisible_ = true;
        dirty_ = true;
    }
}

void UserInterface::handlePairingConfirmationTap(int16_t x, int16_t y) {
    if (y < 151 || y > 190) {
        return;
    }

    if (x >= 36 && x <= 150) {
        pairingConfirmationVisible_ = false;
        dirty_ = true;
        return;
    }

    if (x >= 170 && x <= 284) {
        // Ponowne parowanie, nawet z tym samym hostem, wymaga ponownego
        // potwierdzenia systemu operacyjnego po ustanowieniu nowego bond.
        configManager_.data().pairedHostId = "";
        configManager_.save();
        M5.Display.fillScreen(color(9, 14, 24));
        M5.Display.setTextDatum(middle_center);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.setTextFont(2);
        M5.Display.drawString("Usuwam parowanie...", 160, 108);
        M5.Display.drawString("Urzadzenie uruchomi sie ponownie", 160, 135);
        delay(300);
        bleController_.forgetBondAndRestart();
    }
}

void UserInterface::handlePowerMenuTap(int16_t x, int16_t y) {
    if (y < POWER_BUTTONS_Y || y > POWER_BUTTONS_Y + POWER_BUTTONS_HEIGHT) {
        return;
    }

    if (x >= POWER_BUTTON_1_X && x < POWER_BUTTON_1_X + POWER_BUTTON_WIDTH) {
        enterStandby();
        return;
    }

    if (x >= POWER_BUTTON_2_X && x < POWER_BUTTON_2_X + POWER_BUTTON_WIDTH) {
        shutdownDevice();
        return;
    }

    if (x >= POWER_BUTTON_3_X && x < POWER_BUTTON_3_X + POWER_BUTTON_WIDTH) {
        powerMenuVisible_ = false;
        noteActivity();
        dirty_ = true;
    }
}

void UserInterface::handleStandbyPromptTap(int16_t x, int16_t y) {
    if (y < STANDBY_BUTTONS_Y || y > STANDBY_BUTTONS_Y + STANDBY_BUTTONS_HEIGHT) {
        cancelStandbyPrompt();
        return;
    }

    if (x >= STANDBY_CANCEL_X && x < STANDBY_CANCEL_X + STANDBY_BUTTON_WIDTH) {
        cancelStandbyPrompt();
        return;
    }

    if (x >= STANDBY_NOW_X && x < STANDBY_NOW_X + STANDBY_BUTTON_WIDTH) {
        enterStandby();
        return;
    }

    cancelStandbyPrompt();
}

void UserInterface::draw() {
    dirty_ = false;
    statusBarDirty_ = false;

    switch (screen_) {
        case Screen::Clock: drawClockScreen(); break;
        case Screen::Apps: drawAppsScreen(); break;
        case Screen::Media: drawMediaScreen(); break;
        case Screen::Windows: drawWindowsScreen(); break;
        case Screen::Settings: drawSettingsScreen(); break;
        case Screen::Connections: drawConnectionsScreen(); break;
        default: drawClockScreen(); break;
    }

    if (hostOsPromptVisible_) {
        drawHostOSPrompt();
    } else if (macSpotlightPromptVisible_) {
        drawMacSpotlightPrompt();
    } else if (powerMenuVisible_) {
        drawPowerMenu();
    } else if (standbyPromptVisible_) {
        drawStandbyPrompt();
    } else if (pairingConfirmationVisible_) {
        drawPairingConfirmation();
    } else if (!toastMessage_.isEmpty() && static_cast<int32_t>(millis() - toastUntilMillis_) < 0) {
        drawToast();
    }
}

void UserInterface::drawStatusBar(const char* title) {
    const uint16_t barColor = color(7, 11, 20);
    const uint16_t mutedColor = color(126, 143, 166);
    const uint16_t inactivePill = color(47, 57, 73);
    M5.Display.fillRect(0, 0, DISPLAY_WIDTH, STATUS_BAR_HEIGHT, barColor);

    M5.Display.setTextFont(2);
    M5.Display.setTextSize(1);
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextColor(TFT_WHITE, barColor);
    M5.Display.drawString(title, 8, 12);

    const bool wifiConnected = networkPortal_.isWifiConnected();
    M5.Display.fillRoundRect(201, 4, 26, 17, 5, wifiConnected ? color(45, 143, 255) : inactivePill);
    drawWifiStatusIcon(214, 12, wifiConnected ? TFT_WHITE : mutedColor);

    const bool bleConnected = bleController_.isConnected();
    M5.Display.fillRoundRect(232, 4, 26, 17, 5, bleConnected ? color(0, 190, 122) : inactivePill);
    drawBluetoothStatusIcon(245, 12, bleConnected ? TFT_WHITE : mutedColor);

    drawBattery(267, 5, lastBatteryLevel_, lastCharging_);
}

void UserInterface::drawCurrentStatusBar() {
    switch (screen_) {
        case Screen::Clock: drawStatusBar("Zegar"); break;
        case Screen::Apps: drawStatusBar("Aplikacje"); break;
        case Screen::Media: drawStatusBar("Multimedia"); break;
        case Screen::Windows: drawStatusBar(hostOsName(configManager_.data().hostOs)); break;
        case Screen::Settings: drawStatusBar("Ustawienia"); break;
        case Screen::Connections: drawStatusBar("Polaczenia"); break;
        default: drawStatusBar("M5 Control Deck"); break;
    }
}

void UserInterface::drawPageIndicator(uint16_t backgroundColor) {
    M5.Display.fillRect(0, 219, DISPLAY_WIDTH, 21, backgroundColor);
    const int count = static_cast<int>(Screen::Count);
    const int totalWidth = count * 12;
    const int startX = (DISPLAY_WIDTH - totalWidth) / 2 + 6;

    for (int index = 0; index < count; ++index) {
        M5.Display.fillCircle(
            startX + index * 12,
            PAGE_INDICATOR_Y,
            index == static_cast<int>(screen_) ? 4 : 2,
            index == static_cast<int>(screen_) ? color(54, 165, 255) : color(87, 101, 123)
        );
    }
}

void UserInterface::drawClockScreen() {
    const bool online = networkPortal_.isWifiConnected();
    const uint16_t background = online ? color(15, 79, 137) : color(61, 67, 78);
    const uint16_t card = online ? color(20, 94, 158) : color(75, 81, 92);

    M5.Display.fillScreen(background);
    drawStatusBar("Zegar");

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(color(210, 231, 249), background);
    M5.Display.drawString("POLSKA", 160, 37);

    M5.Display.fillRoundRect(20, 47, 280, 58, 12, card);

    M5.Display.setTextColor(color(210, 231, 249), background);
    M5.Display.drawString(timeManager_.secondTimezoneName() + " / " + timeManager_.secondTimezoneShortName(), 160, 119);

    M5.Display.fillRoundRect(20, 126, 280, 58, 12, card);

    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(211, 220, 231), background);
    M5.Display.drawString(
        online ? "Czas aktualizowany przez NTP" : "Tryb offline: czas z RTC",
        160,
        211
    );

    drawPageIndicator(background);

    lastPolishTime_ = "";
    lastSecondTime_ = "";
    lastPolishDate_ = "";
    drawClockValues(true);
}

void UserInterface::drawClockValues(bool force) {
    clockValuesDirty_ = false;

    const bool online = networkPortal_.isWifiConnected();
    const uint16_t background = online ? color(15, 79, 137) : color(61, 67, 78);
    const uint16_t card = online ? color(20, 94, 158) : color(75, 81, 92);

    const String polishTime = timeManager_.polishTime(false);
    const String secondTime = timeManager_.secondTime(false);
    const String polishDate = timeManager_.polishDate();

    if (force || polishTime != lastPolishTime_) {
        drawClockCard(47, polishTime, card, background);
        lastPolishTime_ = polishTime;
    }

    if (force || secondTime != lastSecondTime_) {
        drawClockCard(126, secondTime, card, background);
        lastSecondTime_ = secondTime;
    }

    if (force || polishDate != lastPolishDate_) {
        M5.Display.fillRect(10, 184, 300, 22, background);
        M5.Display.setTextDatum(middle_center);
        M5.Display.setTextFont(2);
        M5.Display.setTextColor(TFT_WHITE, background);
        M5.Display.drawString(polishDate, 160, 195);
        lastPolishDate_ = polishDate;
    }
}

void UserInterface::drawClockCard(int16_t y, const String& value, uint16_t cardColor, uint16_t backgroundColor) {
    if (clockCardCanvasReady_) {
        clockCardCanvas_.fillSprite(backgroundColor);
        clockCardCanvas_.fillRoundRect(0, 0, 280, 58, 12, cardColor);
        clockCardCanvas_.setTextDatum(middle_center);
        clockCardCanvas_.setTextFont(7);
        clockCardCanvas_.setTextColor(TFT_WHITE, cardColor);
        clockCardCanvas_.drawString(value, 140, 29);
        clockCardCanvas_.pushSprite(20, y);
        return;
    }

    // Wariant awaryjny, gdy zabraknie pamieci na bufor ekranu.
    M5.Display.fillRoundRect(20, y, 280, 58, 12, cardColor);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(7);
    M5.Display.setTextColor(TFT_WHITE, cardColor);
    M5.Display.drawString(value, 160, y + 29);
}

void UserInterface::drawAppsScreen() {
    const uint16_t background = color(9, 14, 24);
    M5.Display.fillScreen(background);
    drawStatusBar("Aplikacje");

    static const Icon ICONS[6] = {
        Icon::Chat,
        Icon::Claude,
        Icon::Gemini,
        Icon::Design,
        Icon::AdminTools,
        Icon::Microchip
    };
    static const uint16_t COLORS[6][3] = {
        {22, 163, 121}, {203, 112, 77}, {66, 133, 244},
        {141, 72, 255}, {33, 150, 243}, {255, 122, 0}
    };

    for (int index = 0; index < 6; ++index) {
        const int column = index % 3;
        const int row = index / 3;
        drawTile(
            TILE_X + column * TILE_X_STEP,
            TILE_Y + row * TILE_Y_STEP,
            TILE_WIDTH,
            TILE_HEIGHT,
            ICONS[index],
            configManager_.data().apps[index].label,
            color(COLORS[index][0], COLORS[index][1], COLORS[index][2]),
            color(103, 122, 151)
        );
    }

    drawPageIndicator(background);
}

void UserInterface::drawMediaScreen() {
    const uint16_t background = color(9, 14, 24);
    M5.Display.fillScreen(background);
    drawStatusBar("Multimedia");

    static const Icon ICONS[6] = {
        Icon::Previous,
        Icon::PlayPause,
        Icon::Next,
        Icon::Mute,
        Icon::VolumeDown,
        Icon::VolumeUp
    };
    static const char* LABELS[6] = {"Poprzedni", "Play / Pauza", "Nastepny", "Wycisz", "Ciszej", "Glosniej"};
    static const uint16_t COLORS[6][3] = {
        {34, 110, 188}, {0, 170, 125}, {34, 110, 188},
        {199, 73, 86}, {107, 79, 187}, {107, 79, 187}
    };

    for (int index = 0; index < 6; ++index) {
        const int column = index % 3;
        const int row = index / 3;
        drawTile(
            TILE_X + column * TILE_X_STEP,
            TILE_Y + row * TILE_Y_STEP,
            TILE_WIDTH,
            TILE_HEIGHT,
            ICONS[index],
            LABELS[index],
            color(COLORS[index][0], COLORS[index][1], COLORS[index][2]),
            color(103, 122, 151)
        );
    }

    drawPageIndicator(background);
}

void UserInterface::drawWindowsScreen() {
    const uint16_t background = color(9, 14, 24);
    M5.Display.fillScreen(background);
    drawStatusBar(hostOsName(configManager_.data().hostOs));

    static const Icon ICONS[6] = {
        Icon::TaskManager,
        Icon::Lock,
        Icon::Snip,
        Icon::Desktop,
        Icon::Folder,
        Icon::SwitchWindow
    };
    static const char* LABELS_WIN[6] = {"Menedzer", "Zablokuj", "Wycinek", "Pulpit", "Pliki", "Alt + Tab"};
    static const char* LABELS_MAC[6] = {"Monitor", "Zablokuj", "Zrzut", "Pulpit", "Finder", "Cmd + Tab"};
    static const char* LABELS_GNOME[6] = {"Monitor", "Zablokuj", "Zrzut", "Aktywnosc", "Pliki", "Super+Tab"};
    const HostOS os = configManager_.data().hostOs;
    const char* const* labels = os == HostOS::MacOS ? LABELS_MAC :
        (os == HostOS::LinuxGnome ? LABELS_GNOME : LABELS_WIN);
    static const uint16_t COLORS[6][3] = {
        {42, 130, 218}, {198, 74, 86}, {147, 81, 191},
        {35, 151, 124}, {225, 154, 45}, {55, 111, 168}
    };

    for (int index = 0; index < 6; ++index) {
        const int column = index % 3;
        const int row = index / 3;
        drawTile(
            TILE_X + column * TILE_X_STEP,
            TILE_Y + row * TILE_Y_STEP,
            TILE_WIDTH,
            TILE_HEIGHT,
            ICONS[index],
            labels[index],
            color(COLORS[index][0], COLORS[index][1], COLORS[index][2]),
            color(103, 122, 151)
        );
    }

    drawPageIndicator(background);
}

void UserInterface::drawSettingsScreen() {
    const uint16_t background = color(9, 14, 24);
    M5.Display.fillScreen(background);
    drawStatusBar("Ustawienia");

    drawSettingRow(32, "Jasnosc", String(configManager_.data().brightness), true, false, false);
    drawSettingRow(76, "Wi-Fi domowe", configManager_.data().wifiEnabled ? "WL." : "WYL.", false, true, configManager_.data().wifiEnabled);
    drawSettingRow(120, "AP konfiguracji", networkPortal_.isApActive() ? "WL." : "WYL.", false, true, networkPortal_.isApActive());
    drawSettingRow(164, "Drugi zegar", timeManager_.secondTimezoneShortName(), true, false, false);

    M5.Display.setTextFont(1);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextColor(color(153, 170, 194), background);
    M5.Display.drawString(
        networkPortal_.isApActive() ? "Panel: 192.168.4.1" : "AP wlaczysz dotykajac trzeciego wiersza",
        160,
        211
    );

    drawPageIndicator(background);
}

void UserInterface::drawConnectionsScreen() {
    const uint16_t background = color(9, 14, 24);
    const uint16_t card = color(23, 32, 51);
    const uint16_t accent = color(37, 114, 190);
    M5.Display.fillScreen(background);
    drawStatusBar("Polaczenia");

    M5.Display.fillRoundRect(12, 31, 296, 44, 9, card);
    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, card);
    M5.Display.drawString("Bluetooth", 23, 44);
    M5.Display.setTextColor(bleController_.isConnected() ? color(0, 210, 135) : color(235, 171, 70), card);
    M5.Display.drawString(bleController_.isConnected() ? "Polaczono" : "Oczekiwanie na komputer", 23, 62);

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(170, 190, 217), background);
    M5.Display.drawString("SYSTEM KOMPUTERA - WYBIERZ", 160, 87);
    static const char* OS_LABELS[] = {"Windows", "macOS", "Linux"};
    for (int i = 0; i < 3; ++i) {
        const int left = 12 + 101 * i;
        const bool chosen = static_cast<uint8_t>(configManager_.data().hostOs) == i;
        const uint16_t fill = chosen ? accent : card;
        M5.Display.fillRoundRect(left, 99, 96, 39, 8, fill);
        if (chosen) M5.Display.drawRoundRect(left, 99, 96, 39, 8, color(96, 181, 252));
        M5.Display.setTextColor(TFT_WHITE, fill);
        M5.Display.setTextFont(2);
        M5.Display.drawString(OS_LABELS[i], left + 48, 118);
    }

    M5.Display.fillRoundRect(18, 151, 284, 36, 8, accent);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, accent);
    M5.Display.drawString("NOWE PAROWANIE BLUETOOTH", 160, 169);

    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(172, 191, 217), background);
    M5.Display.drawString(
        networkPortal_.isApActive() ? "AP: 192.168.4.1 / admin" : "AP: wlacz w Ustawieniach",
        160, 200
    );
    M5.Display.setTextDatum(middle_right);
    M5.Display.setTextColor(color(126, 143, 166), background);
    M5.Display.drawString(String("v") + UserDefaults::FIRMWARE_VERSION, 307, 215);
    drawPageIndicator(background);
}

void UserInterface::drawHostOSPrompt() {
    const uint16_t background = color(9, 14, 24);
    const uint16_t card = color(23, 32, 51);
    M5.Display.fillScreen(background);
    drawStatusBar("Nowy komputer");
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, background);
    M5.Display.drawString("Wybierz system komputera", 160, 49);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(170, 190, 217), background);
    M5.Display.drawString("Nowe parowanie Bluetooth", 160, 72);
    M5.Display.drawString("Wybierz zestaw skrotow dla hosta.", 160, 87);
    static const char* OS_LABELS[] = {"Windows", "macOS", "Linux"};
    for (int i = 0; i < 3; ++i) {
        const int left = 10 + i * 102;
        M5.Display.fillRoundRect(left, 108, 96, 61, 10, card);
        M5.Display.drawRoundRect(left, 108, 96, 61, 10, color(84, 144, 202));
        M5.Display.setTextFont(2);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString(OS_LABELS[i], left + 48, 133);
        M5.Display.setTextFont(1);
        M5.Display.drawString(i == 2 ? "GNOME" : "", left + 48, 152);
    }
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(155, 175, 201), background);
    M5.Display.drawString("Mozesz pozniej zmienic w Polaczeniach", 160, 197);
    M5.Display.drawString("lub w lokalnym panelu WWW.", 160, 210);
}

void UserInterface::drawMacSpotlightPrompt() {
    const uint16_t background = color(10, 17, 29);
    const uint16_t panel = color(23, 35, 56);
    M5.Display.fillScreen(background);
    drawStatusBar("Test Spotlight");
    M5.Display.fillRoundRect(12, 34, 296, 189, 14, panel);
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextColor(TFT_WHITE, panel);
    M5.Display.setTextFont(2);
    M5.Display.drawString("Czy otwarto Spotlight?", 160, 62);
    M5.Display.setTextFont(1);
    M5.Display.drawString("Sprawdz ekran Maca po Cmd+Spacja.", 160, 91);
    M5.Display.drawString("NIE = nie wpisuj tekstu do edytora", 160, 110);
    M5.Display.drawString("TAK = wykonaj akcje i zapamietaj", 160, 125);
    M5.Display.fillRoundRect(20, 152, 129, 46, 9, color(85, 93, 111));
    M5.Display.fillRoundRect(170, 152, 129, 46, 9, color(31, 130, 92));
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextFont(2);
    M5.Display.drawString("NIE", 84, 174);
    M5.Display.drawString("TAK", 235, 174);
}

void UserInterface::drawPairingConfirmation() {
    const uint16_t panel = color(20, 29, 45);
    M5.Display.fillRoundRect(18, 49, 284, 143, 14, panel);
    M5.Display.drawRoundRect(18, 49, 284, 143, 14, color(75, 151, 226));

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, panel);
    M5.Display.drawString("Rozpoczac nowe parowanie?", 160, 70);

    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(188, 201, 218), panel);
    M5.Display.drawString("Panel zapisuje tylko jedno powiazanie BLE.", 160, 94);
    M5.Display.drawString("Usun M5 Control Deck z poprzedniego hosta", 160, 109);
    M5.Display.drawString("przed ponownym parowaniem.", 160, 122);

    M5.Display.fillRoundRect(36, 151, 114, 38, 9, color(61, 72, 90));
    M5.Display.fillRoundRect(170, 151, 114, 38, 9, color(37, 114, 190));
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("ANULUJ", 93, 170);
    M5.Display.drawString("PARUJ", 227, 170);
}

void UserInterface::drawPowerMenu() {
    const uint16_t panel = color(18, 27, 43);
    const uint16_t card = color(31, 43, 64);
    const uint16_t accent = color(54, 153, 238);
    const uint16_t danger = color(204, 71, 89);

    M5.Display.fillRoundRect(14, 39, 292, 166, 15, panel);
    M5.Display.drawRoundRect(14, 39, 292, 166, 15, color(64, 91, 126));

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, panel);
    M5.Display.drawString("Zasilanie", 160, 58);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(159, 177, 202), panel);
    M5.Display.drawString("Wybierz tryb pracy urzadzenia", 160, 76);

    M5.Display.fillCircle(65, 108, 22, card);
    M5.Display.drawCircle(65, 108, 22, accent);
    M5.Display.drawCircle(61, 105, 10, accent);
    M5.Display.fillCircle(67, 101, 10, card);

    M5.Display.fillCircle(160, 108, 22, card);
    M5.Display.drawCircle(160, 108, 18, danger);
    M5.Display.fillRect(157, 87, 6, 20, card);
    M5.Display.drawLine(160, 87, 160, 104, danger);
    M5.Display.fillCircle(160, 87, 2, danger);

    M5.Display.fillCircle(255, 108, 22, card);
    M5.Display.drawCircle(255, 108, 22, color(132, 149, 173));
    drawThickLine(247, 100, 263, 116, color(170, 186, 208), 2);
    drawThickLine(263, 100, 247, 116, color(170, 186, 208), 2);

    M5.Display.fillRoundRect(POWER_BUTTON_1_X, POWER_BUTTONS_Y, POWER_BUTTON_WIDTH, POWER_BUTTONS_HEIGHT, 9, accent);
    M5.Display.fillRoundRect(POWER_BUTTON_2_X, POWER_BUTTONS_Y, POWER_BUTTON_WIDTH, POWER_BUTTONS_HEIGHT, 9, danger);
    M5.Display.fillRoundRect(POWER_BUTTON_3_X, POWER_BUTTONS_Y, POWER_BUTTON_WIDTH, POWER_BUTTONS_HEIGHT, 9, color(61, 73, 93));
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("CZUWANIE", 65, POWER_BUTTONS_Y + 21);
    M5.Display.drawString("WYLACZ", 160, POWER_BUTTONS_Y + 21);
    M5.Display.drawString("ANULUJ", 255, POWER_BUTTONS_Y + 21);
}

void UserInterface::drawStandbyPrompt() {
    const uint16_t panel = color(18, 28, 45);
    const uint16_t accent = color(53, 155, 242);
    const uint16_t secondary = color(61, 73, 93);
    const uint8_t remaining = standbyCountdownRemaining();

    M5.Display.fillRoundRect(20, 47, 280, 149, 15, panel);
    M5.Display.drawRoundRect(20, 47, 280, 149, 15, color(67, 122, 178));

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, panel);
    M5.Display.drawString("Oszczedzanie energii", 160, 68);

    M5.Display.setTextFont(1);
    M5.Display.setTextColor(color(173, 190, 214), panel);
    M5.Display.drawString("Bluetooth nie jest polaczony.", 160, 91);
    M5.Display.drawString("Ekran przejdzie w czuwanie za", 160, 107);

    M5.Display.fillCircle(160, 130, 19, color(29, 48, 72));
    M5.Display.drawCircle(160, 130, 19, accent);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, color(29, 48, 72));
    M5.Display.drawString(String(remaining), 160, 130);

    M5.Display.fillRoundRect(STANDBY_CANCEL_X, STANDBY_BUTTONS_Y, STANDBY_BUTTON_WIDTH, STANDBY_BUTTONS_HEIGHT, 9, secondary);
    M5.Display.fillRoundRect(STANDBY_NOW_X, STANDBY_BUTTONS_Y, STANDBY_BUTTON_WIDTH, STANDBY_BUTTONS_HEIGHT, 9, accent);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawString("ANULUJ", STANDBY_CANCEL_X + STANDBY_BUTTON_WIDTH / 2, STANDBY_BUTTONS_Y + 20);
    M5.Display.drawString("TERAZ", STANDBY_NOW_X + STANDBY_BUTTON_WIDTH / 2, STANDBY_BUTTONS_Y + 20);
}

void UserInterface::drawToast() {
    const uint16_t toastColor = color(18, 27, 42);
    M5.Display.fillRoundRect(24, 190, 272, 32, 10, toastColor);
    M5.Display.drawRoundRect(24, 190, 272, 32, 10, color(67, 145, 220));
    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(TFT_WHITE, toastColor);
    M5.Display.drawString(toastMessage_, 160, 206);
}

void UserInterface::drawTile(
    int16_t x,
    int16_t y,
    int16_t width,
    int16_t height,
    Icon icon,
    const String& label,
    uint16_t fillColor,
    uint16_t borderColor
) {
    M5.Display.fillRoundRect(x, y, width, height, 12, fillColor);
    M5.Display.drawRoundRect(x, y, width, height, 12, borderColor);

    drawIcon(icon, x + width / 2, y + 29, TFT_WHITE, fillColor);

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(TFT_WHITE, fillColor);

    String firstLine = label;
    String secondLine;
    if (label.length() > 12) {
        int split = label.lastIndexOf(' ', 12);
        if (split <= 0) split = label.indexOf(' ');
        if (split > 0) {
            firstLine = label.substring(0, split);
            secondLine = label.substring(split + 1);
        } else {
            firstLine = label.substring(0, 12);
            secondLine = label.substring(12);
        }
    }

    if (firstLine.length() > 12) {
        firstLine = firstLine.substring(0, 11) + "~";
    }
    if (secondLine.length() > 12) {
        secondLine = secondLine.substring(0, 11) + "~";
    }

    if (secondLine.isEmpty()) {
        M5.Display.drawString(firstLine, x + width / 2, y + height - 18);
    } else {
        M5.Display.drawString(firstLine, x + width / 2, y + height - 25);
        M5.Display.drawString(secondLine, x + width / 2, y + height - 12);
    }
}

void UserInterface::drawIcon(
    Icon icon,
    int16_t centerX,
    int16_t centerY,
    uint16_t foreground,
    uint16_t background
) {
    switch (icon) {
        case Icon::Chat: {
            M5.Display.drawRoundRect(centerX - 20, centerY - 14, 40, 27, 7, foreground);
            M5.Display.drawRoundRect(centerX - 19, centerY - 13, 38, 25, 6, foreground);
            drawThickLine(centerX - 9, centerY + 12, centerX - 14, centerY + 18, foreground, 3);
            M5.Display.fillCircle(centerX - 9, centerY, 3, foreground);
            M5.Display.fillCircle(centerX, centerY, 3, foreground);
            M5.Display.fillCircle(centerX + 9, centerY, 3, foreground);
            break;
        }
        case Icon::Claude: {
            M5.Display.fillCircle(centerX, centerY, 5, foreground);
            for (int16_t offset = -1; offset <= 1; ++offset) {
                M5.Display.drawLine(centerX - 18, centerY + offset, centerX - 8, centerY + offset, foreground);
                M5.Display.drawLine(centerX + 8, centerY + offset, centerX + 18, centerY + offset, foreground);
                M5.Display.drawLine(centerX + offset, centerY - 18, centerX + offset, centerY - 8, foreground);
                M5.Display.drawLine(centerX + offset, centerY + 8, centerX + offset, centerY + 18, foreground);
            }
            drawThickLine(centerX - 13, centerY - 13, centerX - 7, centerY - 7, foreground, 3);
            drawThickLine(centerX + 7, centerY + 7, centerX + 13, centerY + 13, foreground, 3);
            drawThickLine(centerX + 13, centerY - 13, centerX + 7, centerY - 7, foreground, 3);
            drawThickLine(centerX - 7, centerY + 7, centerX - 13, centerY + 13, foreground, 3);
            break;
        }
        case Icon::Gemini: {
            M5.Display.fillTriangle(centerX, centerY - 20, centerX - 5, centerY, centerX + 5, centerY, foreground);
            M5.Display.fillTriangle(centerX, centerY + 20, centerX - 5, centerY, centerX + 5, centerY, foreground);
            M5.Display.fillTriangle(centerX - 20, centerY, centerX, centerY - 5, centerX, centerY + 5, foreground);
            M5.Display.fillTriangle(centerX + 20, centerY, centerX, centerY - 5, centerX, centerY + 5, foreground);
            M5.Display.fillCircle(centerX + 15, centerY - 13, 3, foreground);
            break;
        }
        case Icon::Design: {
            M5.Display.drawRoundRect(centerX - 20, centerY - 15, 40, 30, 6, foreground);
            M5.Display.drawRoundRect(centerX - 19, centerY - 14, 38, 28, 5, foreground);
            drawThickLine(centerX - 10, centerY + 9, centerX + 11, centerY - 12, foreground, 4);
            M5.Display.fillTriangle(centerX + 9, centerY - 14, centerX + 15, centerY - 15, centerX + 14, centerY - 8, foreground);
            M5.Display.fillCircle(centerX - 11, centerY - 8, 3, foreground);
            break;
        }
        case Icon::AdminTools: {
            M5.Display.drawRoundRect(centerX - 21, centerY - 15, 42, 30, 5, foreground);
            drawThickLine(centerX - 21, centerY - 7, centerX + 21, centerY - 7, foreground, 2);
            M5.Display.fillCircle(centerX - 14, centerY - 11, 2, foreground);
            M5.Display.fillCircle(centerX - 7, centerY - 11, 2, foreground);
            drawThickLine(centerX - 12, centerY, centerX - 6, centerY + 5, foreground, 3);
            drawThickLine(centerX - 6, centerY + 5, centerX - 12, centerY + 10, foreground, 3);
            drawThickLine(centerX, centerY + 10, centerX + 12, centerY + 10, foreground, 3);
            break;
        }
        case Icon::Microchip: {
            M5.Display.drawRoundRect(centerX - 14, centerY - 14, 28, 28, 5, foreground);
            M5.Display.drawRoundRect(centerX - 12, centerY - 12, 24, 24, 4, foreground);
            for (int offset = -9; offset <= 9; offset += 9) {
                drawThickLine(centerX - 20, centerY + offset, centerX - 15, centerY + offset, foreground, 2);
                drawThickLine(centerX + 15, centerY + offset, centerX + 20, centerY + offset, foreground, 2);
                drawThickLine(centerX + offset, centerY - 20, centerX + offset, centerY - 15, foreground, 2);
                drawThickLine(centerX + offset, centerY + 15, centerX + offset, centerY + 20, foreground, 2);
            }
            M5.Display.fillCircle(centerX, centerY, 5, foreground);
            break;
        }
        case Icon::Previous: {
            M5.Display.fillRect(centerX - 21, centerY - 15, 4, 30, foreground);
            M5.Display.fillTriangle(centerX - 15, centerY, centerX, centerY - 14, centerX, centerY + 14, foreground);
            M5.Display.fillTriangle(centerX, centerY, centerX + 15, centerY - 14, centerX + 15, centerY + 14, foreground);
            break;
        }
        case Icon::PlayPause: {
            M5.Display.fillTriangle(centerX - 20, centerY - 16, centerX - 20, centerY + 16, centerX - 1, centerY, foreground);
            M5.Display.fillRoundRect(centerX + 5, centerY - 15, 6, 30, 2, foreground);
            M5.Display.fillRoundRect(centerX + 15, centerY - 15, 6, 30, 2, foreground);
            break;
        }
        case Icon::Next: {
            M5.Display.fillTriangle(centerX - 15, centerY - 14, centerX - 15, centerY + 14, centerX, centerY, foreground);
            M5.Display.fillTriangle(centerX, centerY - 14, centerX, centerY + 14, centerX + 15, centerY, foreground);
            M5.Display.fillRect(centerX + 17, centerY - 15, 4, 30, foreground);
            break;
        }
        case Icon::Mute:
        case Icon::VolumeDown:
        case Icon::VolumeUp: {
            // Klasyczny glosnik sklada sie z malego korpusu i krotszego stozka.
            // Dwa skosne ramiona maja identyczna dlugosc, a pionowa krawedz
            // stozka jest wyraznie dluzsza. Fale dzwieku sa odsuniete od bryly,
            // dzieki czemu nie nachodza na nia nawet przy grubosci trzech pikseli.
            constexpr int16_t bodyLeft = -20;
            constexpr int16_t bodyRight = -12;
            constexpr int16_t bodyHalfHeight = 7;
            constexpr int16_t coneNearX = -12;
            constexpr int16_t coneFarX = -4;
            constexpr int16_t coneNearHalfHeight = 7;
            constexpr int16_t coneFarHalfHeight = 13;

            M5.Display.fillRoundRect(
                centerX + bodyLeft,
                centerY - bodyHalfHeight,
                bodyRight - bodyLeft + 1,
                bodyHalfHeight * 2 + 1,
                2,
                foreground
            );

            // Wypelniony trapez daje czytelniejsza sylwetke niz duzy trojkat.
            M5.Display.fillTriangle(
                centerX + coneNearX,
                centerY - coneNearHalfHeight,
                centerX + coneFarX,
                centerY - coneFarHalfHeight,
                centerX + coneFarX,
                centerY + coneFarHalfHeight,
                foreground
            );
            M5.Display.fillTriangle(
                centerX + coneNearX,
                centerY - coneNearHalfHeight,
                centerX + coneFarX,
                centerY + coneFarHalfHeight,
                centerX + coneNearX,
                centerY + coneNearHalfHeight,
                foreground
            );

            auto drawSoundWave = [&](int16_t startX, int16_t peakX, int16_t halfHeight) {
                drawThickLine(
                    centerX + startX,
                    centerY - halfHeight,
                    centerX + peakX,
                    centerY,
                    foreground,
                    3
                );
                drawThickLine(
                    centerX + peakX,
                    centerY,
                    centerX + startX,
                    centerY + halfHeight,
                    foreground,
                    3
                );
                // Zaokraglenie laczenia obu ramion fali.
                M5.Display.fillCircle(centerX + peakX, centerY, 1, foreground);
            };

            if (icon == Icon::Mute) {
                drawThickLine(centerX + 4, centerY - 8, centerX + 19, centerY + 8, foreground, 3);
                drawThickLine(centerX + 19, centerY - 8, centerX + 4, centerY + 8, foreground, 3);
            } else {
                drawSoundWave(3, 8, 6);
                if (icon == Icon::VolumeUp) {
                    drawSoundWave(12, 20, 11);
                }
            }
            break;
        }
        case Icon::TaskManager: {
            M5.Display.drawRoundRect(centerX - 21, centerY - 16, 42, 32, 5, foreground);
            drawThickLine(centerX - 13, centerY + 9, centerX - 13, centerY + 1, foreground, 4);
            drawThickLine(centerX, centerY + 9, centerX, centerY - 7, foreground, 4);
            drawThickLine(centerX + 13, centerY + 9, centerX + 13, centerY - 12, foreground, 4);
            break;
        }
        case Icon::Lock: {
            M5.Display.drawCircle(centerX, centerY - 8, 10, foreground);
            M5.Display.drawCircle(centerX, centerY - 8, 9, foreground);
            M5.Display.fillRect(centerX - 12, centerY - 8, 24, 10, background);
            M5.Display.fillRoundRect(centerX - 16, centerY - 3, 32, 23, 5, foreground);
            M5.Display.fillCircle(centerX, centerY + 7, 3, background);
            M5.Display.fillRect(centerX - 1, centerY + 8, 3, 7, background);
            break;
        }
        case Icon::Snip: {
            M5.Display.drawCircle(centerX - 11, centerY + 10, 6, foreground);
            M5.Display.drawCircle(centerX + 11, centerY + 10, 6, foreground);
            drawThickLine(centerX - 7, centerY + 5, centerX + 15, centerY - 14, foreground, 3);
            drawThickLine(centerX + 7, centerY + 5, centerX - 15, centerY - 14, foreground, 3);
            M5.Display.fillCircle(centerX, centerY, 3, foreground);
            break;
        }
        case Icon::Desktop: {
            M5.Display.drawRoundRect(centerX - 22, centerY - 16, 44, 29, 5, foreground);
            M5.Display.fillRect(centerX - 2, centerY + 13, 4, 7, foreground);
            M5.Display.fillRoundRect(centerX - 12, centerY + 19, 24, 4, 2, foreground);
            break;
        }
        case Icon::Folder: {
            M5.Display.fillRoundRect(centerX - 22, centerY - 11, 44, 28, 5, foreground);
            M5.Display.fillRoundRect(centerX - 18, centerY - 17, 18, 10, 4, foreground);
            M5.Display.fillRoundRect(centerX - 18, centerY - 6, 36, 18, 3, background);
            break;
        }
        case Icon::SwitchWindow: {
            M5.Display.drawRoundRect(centerX - 21, centerY - 14, 31, 25, 4, foreground);
            M5.Display.drawRoundRect(centerX - 8, centerY - 6, 29, 24, 4, foreground);
            drawThickLine(centerX - 17, centerY + 18, centerX + 10, centerY + 18, foreground, 3);
            M5.Display.fillTriangle(centerX + 10, centerY + 13, centerX + 18, centerY + 18, centerX + 10, centerY + 23, foreground);
            break;
        }
    }
}

void UserInterface::drawWifiStatusIcon(int16_t centerX, int16_t centerY, uint16_t iconColor) {
    // Prosta ikona Wi-Fi zbudowana z grubych odcinkow, bez zewnetrznych bitmap.
    M5.Display.fillCircle(centerX, centerY + 5, 2, iconColor);

    drawThickLine(centerX - 5, centerY + 1, centerX - 2, centerY - 2, iconColor, 2);
    drawThickLine(centerX - 2, centerY - 2, centerX, centerY - 3, iconColor, 2);
    drawThickLine(centerX, centerY - 3, centerX + 2, centerY - 2, iconColor, 2);
    drawThickLine(centerX + 2, centerY - 2, centerX + 5, centerY + 1, iconColor, 2);

    drawThickLine(centerX - 8, centerY - 3, centerX - 4, centerY - 6, iconColor, 2);
    drawThickLine(centerX - 4, centerY - 6, centerX, centerY - 7, iconColor, 2);
    drawThickLine(centerX, centerY - 7, centerX + 4, centerY - 6, iconColor, 2);
    drawThickLine(centerX + 4, centerY - 6, centerX + 8, centerY - 3, iconColor, 2);
}

void UserInterface::drawBluetoothStatusIcon(int16_t centerX, int16_t centerY, uint16_t iconColor) {
    // Symbol Bluetooth miesci sie w polu 13 x 17 pikseli i pozostaje czytelny
    // zarowno w stanie aktywnym, jak i wyszarzonym.
    drawThickLine(centerX, centerY - 8, centerX, centerY + 8, iconColor, 2);
    drawThickLine(centerX, centerY - 8, centerX + 6, centerY - 3, iconColor, 2);
    drawThickLine(centerX + 6, centerY - 3, centerX, centerY + 1, iconColor, 2);
    drawThickLine(centerX, centerY - 1, centerX + 6, centerY + 4, iconColor, 2);
    drawThickLine(centerX + 6, centerY + 4, centerX, centerY + 8, iconColor, 2);
    drawThickLine(centerX - 5, centerY - 4, centerX + 6, centerY + 4, iconColor, 2);
    drawThickLine(centerX - 5, centerY + 4, centerX + 6, centerY - 3, iconColor, 2);
}

void UserInterface::drawThickLine(
    int16_t x0,
    int16_t y0,
    int16_t x1,
    int16_t y1,
    uint16_t colorValue,
    uint8_t thickness
) {
    const int16_t half = static_cast<int16_t>(thickness / 2);
    const bool horizontalDominant = std::abs(x1 - x0) >= std::abs(y1 - y0);
    for (int16_t offset = -half; offset <= half; ++offset) {
        if (horizontalDominant) {
            M5.Display.drawLine(x0, y0 + offset, x1, y1 + offset, colorValue);
        } else {
            M5.Display.drawLine(x0 + offset, y0, x1 + offset, y1, colorValue);
        }
    }
}

void UserInterface::drawSettingRow(
    int16_t y,
    const String& label,
    const String& value,
    bool showMinusPlus,
    bool showToggle,
    bool toggleValue
) {
    const uint16_t card = color(23, 32, 51);
    M5.Display.fillRoundRect(8, y, 304, 39, 9, card);

    M5.Display.setTextDatum(middle_left);
    M5.Display.setTextFont(2);
    M5.Display.setTextColor(TFT_WHITE, card);
    M5.Display.drawString(label, 18, y + 19);

    if (showToggle) {
        drawToggle(258, y + 8, toggleValue);
        return;
    }

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextColor(color(93, 180, 255), card);
    M5.Display.drawString(value, 180, y + 19);

    if (showMinusPlus) {
        M5.Display.fillRoundRect(211, y + 5, 42, 29, 8, color(54, 68, 91));
        M5.Display.fillRoundRect(264, y + 5, 42, 29, 8, color(54, 68, 91));
        M5.Display.setTextFont(4);
        M5.Display.setTextColor(TFT_WHITE);
        M5.Display.drawString("-", 232, y + 19);
        M5.Display.drawString("+", 285, y + 19);
    }
}

void UserInterface::drawToggle(int16_t x, int16_t y, bool enabled) {
    const uint16_t track = enabled ? color(0, 181, 119) : color(68, 80, 99);
    M5.Display.fillRoundRect(x, y, 45, 23, 12, track);
    M5.Display.fillCircle(enabled ? x + 33 : x + 12, y + 11, 8, TFT_WHITE);
}

void UserInterface::drawBattery(int16_t x, int16_t y, int8_t level, bool charging) {
    const uint16_t outline = color(173, 188, 207);
    M5.Display.drawRoundRect(x, y, 40, 15, 3, outline);
    M5.Display.fillRect(x + 40, y + 5, 3, 5, outline);

    if (level >= 0) {
        const int fillWidth = map(constrain(static_cast<int>(level), 0, 100), 0, 100, 0, 34);
        const uint16_t fill = level > 25 ? color(0, 194, 125) : color(232, 93, 93);
        M5.Display.fillRoundRect(x + 3, y + 3, fillWidth, 9, 2, fill);
    }

    M5.Display.setTextDatum(middle_center);
    M5.Display.setTextFont(1);
    M5.Display.setTextColor(TFT_WHITE);
    const String text = charging ? "+" : (level >= 0 ? String(level) : String("?"));
    M5.Display.drawString(text, x + 20, y + 7);
}

void UserInterface::showToast(const String& message, uint32_t durationMs) {
    toastMessage_ = message;
    toastUntilMillis_ = millis() + durationMs;
    dirty_ = true;
}

bool UserInterface::requireBluetooth() {
    if (hostOsPromptVisible_) {
        showToast("Najpierw wybierz system komputera");
        return false;
    }
    if (bleController_.isConnected()) {
        return true;
    }
    showToast("Najpierw polacz Bluetooth");
    return false;
}

bool UserInterface::sendEncoderAction(bool increaseVolume) {
    const uint32_t now = millis();
    const bool sent = increaseVolume
        ? bleController_.mediaVolumeUp()
        : bleController_.mediaVolumeDown();

    if (!sent && now - lastEncoderDisconnectedToastMillis_ >= 1500) {
        showToast("Najpierw polacz Bluetooth");
        lastEncoderDisconnectedToastMillis_ = now;
    }
    return sent;
}

uint16_t UserInterface::color(uint8_t red, uint8_t green, uint8_t blue) {
    return static_cast<uint16_t>(((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3));
}

int8_t UserInterface::readBatteryLevel() {
    const int8_t level = M5.Power.getBatteryLevel();
    if (level < 0 || level > 100) {
        return -1;
    }
    return level;
}
