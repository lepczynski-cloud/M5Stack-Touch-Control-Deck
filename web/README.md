# M5Stack Touch Control Deck | strona WWW i automatyczny instalator (PL/EN)

Strona statyczna jest częścią **tego samego repozytorium** co firmware.
Nie wymaga backendu, Node.js, osobnego projektu ani GitHub Actions.
**Nie musisz ręcznie kompilować plików `.bin` ani uruchamiać `build_web_firmware.bat` po każdej zmianie kodu.**

## Konfiguracja Cloudflare Pages (jednorazowo)

1. Dodaj zawartość paczki do `lepczynski-cloud/M5Stack-Touch-Control-Deck` (gałąź `main`). Zachowaj folder `images` ze zdjęciem `m5stack-touch-control-deck.jpg`.
2. W Cloudflare **Workers & Pages > Create > Pages > Connect to Git** podłącz **to samo repozytorium GitHub**.
3. Ustaw:

   | Ustawienie | Wartość |
   |---|---|
   | Framework preset | `None` |
   | Production branch | `main` |
   | Root directory | domyślny katalog główny repozytorium |
   | **Build command** | **`sh tools/build_web_firmware.sh`** |
   | **Build output directory** | **`web`** |
   | Python | domyślna wersja Cloudflare; opcjonalnie `PYTHON_VERSION=3.11.11` |

4. Po udanym wdrożeniu dodaj subdomenę `m5stack-deck.lepczynski.it` w **Custom domains**.
5. Sprawdź `https://m5stack-deck.lepczynski.it/?lang=pl` i `?lang=en`.

Cloudflare automatycznie wykonuje build przy każdej zmianie wypchniętej do podłączonej gałęzi. Skrypt:

- tworzy środowisko Python **tylko na czas budowania strony**,
- instaluje określoną wersję PlatformIO (`6.2.0`) i `intelhex`,
- kompiluje działające źródła dla `cores3_se` (`platformio.ini`),
- łączy obraz bootloadera, tablicy partycji i aplikacji,
- generuje `web/firmware/m5stack-touch-control-deck.factory.bin` i `manifest.json`,
- dopiero po sukcesie pozwala Cloudflare opublikować folder `web`.

**Użytkownik końcowy niczego nie kompiluje.** Wybiera tylko port USB w obsługiwanej przeglądarce. Generowany obraz `.bin` powstaje podczas budowania Cloudflare i **nie musi być zapisany w GitHubie**.

### Co jeśli wdrożenie się nie uda?

Cloudflare Pages ma limit **20 minut na pojedynczą kompilację** (również na pobranie zależności za pierwszym razem). Jeśli pobieranie bibliotek lub kompilowanie potrwa dłużej, deployment się nie powiedzie, a poprzednio opublikowana wersja strony pozostanie aktywna. Sprawdź logi w **Workers & Pages > [projekt] > Deployments**.

Przy długotrwałych przekroczeniach limitu można przenieść sam etap kompilacji do GitHub Actions; na start **nie ma potrzeby dodawania workflow**.

## Wgrywanie przez odwiedzającego

1. Podłącz **M5Stack CoreS3 SE** przez USB-C z transmisją danych.
2. Otwórz stronę przez HTTPS w Chrome lub Edge na komputerze.
3. Kliknij **Install firmware / Wgraj firmware** i wybierz port USB urządzenia.
4. Po restarcie sparuj `M5 Control Deck` w Windows 10/11 przez Bluetooth.
5. Konfiguracja stron, Wi-Fi, jasności i stref czasowych odbywa się z lokalnego portalu WWW urządzenia.

Firmware działa jako kontroler **Windows 10/11**. Komputer użyty do wgrania może mieć inny system, jeżeli przeglądarka obsługuje Web Serial. Nie próbuj instalować firmware na innych płytkach ESP32-S3.

**Uwaga:** Flashowanie może usunąć zapisane ustawienia. Dane i hasła z domyślnego pliku `include/UserConfig.h` trafiają do publicznego firmware, więc nie wprowadzaj tam prywatnych danych dostępowych. Konfiguruj je po wgraniu, w lokalnym portalu urządzenia, i zmień domyślne hasła.

## Materiały

- Film z budowy jest osadzony automatycznie: https://www.youtube.com/watch?v=Jz1xCBi-Q1o
- Szczegółowy poradnik po polsku na Forbocie: https://forbot.pl/forum/topic/26182-m5stack-control-deck-bezprzewodowy-panel-sterowania-komputerem-na-cores3-se-bez-lutowania/
- Repozytorium: https://github.com/lepczynski-cloud/M5Stack-Touch-Control-Deck
- Obudowa 3D (projekt innego autora): https://makerworld.com/en/models/3128546-m5stack-core-2-case-45deg-angle-w-unit-holder#profileId-3530280

## Opcjonalne narzędzia lokalne

Skrypty `tools/build_windows.bat`, `tools/upload_windows.bat` i `tools/build_web_firmware.bat` nadal działają jako awaryjna ścieżka dla autora projektu. **Nie są wymagane przy normalnej publikacji przez Cloudflare Pages.**

Źródła: https://developers.cloudflare.com/pages/configuration/build-configuration/ oraz https://esphome.github.io/esp-web-tools/.
