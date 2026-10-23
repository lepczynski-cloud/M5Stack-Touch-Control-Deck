#pragma once

#include <Arduino.h>

// -----------------------------------------------------------------------------
// USTAWIENIA DOMYSLNE PROJEKTU
// -----------------------------------------------------------------------------
// Ten plik mozna edytowac przed pierwszym wgraniem programu.
// Po uruchomieniu te same ustawienia mozna zmienic przez lokalny portal WWW.

namespace UserDefaults {

// Domowa siec Wi-Fi jest opcjonalna. Puste pola uruchomia punkt dostepowy AP.
inline constexpr char WIFI_SSID[] = "";
inline constexpr char WIFI_PASSWORD[] = "";
inline constexpr bool WIFI_ENABLED = true;

// Punkt dostepowy do konfiguracji. Haslo musi miec co najmniej 8 znakow.
inline constexpr char AP_SSID[] = "M5-Control-Deck";
inline constexpr char AP_PASSWORD[] = "m5control";

// Dane logowania do strony konfiguracyjnej.
inline constexpr char WEB_USERNAME[] = "admin";
inline constexpr char WEB_PASSWORD[] = "admin123";

// Nazwa widoczna w ustawieniach Bluetooth komputera.
// Biblioteka BLE ogranicza nazwe urzadzenia do 15 znakow.
inline constexpr char BLE_DEVICE_NAME[] = "M5 Control Deck";
inline constexpr char BLE_MANUFACTURER[] = "Lepczynski";

// Opoznienia uzywane przy otwieraniu adresu przez okno Win+R.
// Mozna je zwiekszyc na wolniejszym komputerze.
inline constexpr uint32_t RUN_DIALOG_DELAY_MS = 450;
inline constexpr uint32_t URL_SUBMIT_DELAY_MS = 100;

// Domyslny system dla dotychczasowych instalacji. Nowy host BLE zawsze
// wymaga potwierdzenia systemu na ekranie albo przez lokalny panel WWW.
inline constexpr uint8_t HOST_OS = 0;  // 0 Windows, 1 macOS, 2 Linux GNOME

// Domyslna jasnosc ekranu w zakresie 20-255.
inline constexpr uint8_t DISPLAY_BRIGHTNESS = 150;

// Domyslna druga strefa czasowa. 0 oznacza HKT.
inline constexpr uint8_t SECOND_TIMEZONE_INDEX = 0;

// Domyslne aplikacje i strony WWW w ukladzie 2 x 3.
inline constexpr char APP_LABELS[6][24] = {
    "ChatGPT",
    "Claude",
    "Gemini",
    "Canva",
    "Admin Toolkit",
    "Botland",
};

inline constexpr char APP_URLS[6][160] = {
    "https://chatgpt.com/",
    "https://claude.ai/",
    "https://gemini.google.com/",
    "https://www.canva.com/",
    "https://admintoolkit.lepczynski.it/",
    "https://botland.com.pl/",
};

// Serwery NTP uzywane do synchronizacji zegara.
inline constexpr char NTP_SERVER_1[] = "pool.ntp.org";
inline constexpr char NTP_SERVER_2[] = "time.cloudflare.com";
inline constexpr char NTP_SERVER_3[] = "time.google.com";

// Parametry sprzetowe modulu Scroll podlaczonego do Port A CoreS3 SE.
// Pokretlo reguluje glosnosc, a jego nacisniecie wysyla Play/Pauza.
inline constexpr uint8_t SCROLL_I2C_ADDRESS = 0x40;
inline constexpr uint8_t SCROLL_SDA_PIN = 2;
inline constexpr uint8_t SCROLL_SCL_PIN = 1;
inline constexpr uint32_t SCROLL_I2C_SPEED = 400000;
inline constexpr bool SCROLL_REVERSE_DIRECTION = false;

// Automatyczne oszczedzanie energii dziala tylko wtedy, gdy Bluetooth nie jest
// polaczony. Po tym czasie pojawi sie pytanie, a po odliczeniu ekran przejdzie
// w tryb czuwania. Bluetooth i Wi-Fi nadal dzialaja w tle.
inline constexpr bool STANDBY_ENABLED = true;
inline constexpr uint16_t STANDBY_TIMEOUT_MINUTES = 5;
inline constexpr uint8_t STANDBY_COUNTDOWN_SECONDS = 12;

// Po tym czasie nieuzywany punkt AP zostanie automatycznie wylaczony.
inline constexpr uint32_t AP_IDLE_TIMEOUT_MS = 10UL * 60UL * 1000UL;

// Czas oczekiwania na polaczenie z zapisanym Wi-Fi.
inline constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15UL * 1000UL;

inline constexpr char FIRMWARE_VERSION[] = "0.2.2";
inline constexpr char PROJECT_NAME[] = "M5Stack Touch Control Deck";

static_assert(sizeof(AP_PASSWORD) - 1 >= 8, "Domyslne haslo AP musi miec co najmniej 8 znakow.");
static_assert(sizeof(BLE_DEVICE_NAME) - 1 <= 15, "Nazwa urzadzenia BLE moze miec maksymalnie 15 znakow.");

}  // namespace UserDefaults
