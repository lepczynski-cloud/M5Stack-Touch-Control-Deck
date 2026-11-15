# Strona projektu i instalator / Project website and installer

Strona w `web/` to lekki, statyczny HTML + CSS + JavaScript, bez bundlera i frameworka. Obsługuje dwa języki (PL/EN), podświetlenie pod kursorem, subtelne animacje przy przewijaniu i instalator ESP Web Tools. Na telefonach i przy `prefers-reduced-motion` efekt podświetlenia/animacje są ograniczone. Film YouTube jest ładowany dopiero po kliknięciu. Nie używamy zewnętrznych fontów, bibliotek animacji ani analityki.

The `web/` directory is a lightweight, bilingual static HTML/CSS/JS website. It includes a cursor-following spotlight, subtle scroll animations, accessible reduced-motion support and ESP Web Tools. The YouTube iframe loads only on click. No font CDN, animation framework or analytics are required.

## Build i publikacja / Build and deployment

**Zachowany dotychczasowy proces automatycznego budowania i publikowania:**

1. `tools/build_web_firmware.sh` tworzy tymczasowe środowisko Pythona i instaluje przypięte zależności.
2. Skrypt kopiuje źródłowe `images/m5stack-touch-control-deck.jpg` do `web/assets/` (kopia jest ignorowana w Git), a następnie kompiluje firmware CoreS3 SE.
3. `tools/package_web_firmware.py` generuje obrazy `.bin` i manifesty na podstawie `include/UserConfig.h`.
4. Hosting statyczny publikuje katalog `./web`, zgodnie z `wrangler.toml`.

Uruchomienie lokalne w repozytorium: `sh tools/build_web_firmware.sh`. Nie należy zastępować tej komendy samą publikacją folderu `web/`, gdyż bez wcześniej wygenerowanych binariów instalator pozostanie nieaktywny. Nie dodawaj zbudowanych firmware `.bin` ani kopii zdjęcia do repozytorium.

The existing automated website-build contract is unchanged: run `sh tools/build_web_firmware.sh` before deploying `./web`. The script builds current firmware, copies the project image, then writes the matching firmware images and manifests. Without those generated files the webpage stays readable, but shows the installer as unavailable rather than offering stale firmware.

## Dwa tryby instalacji / Two installation modes

| Opcja / Mode | Plik / Image | Offset | Ustawienia / Settings |
| --- | --- | --- | --- |
| **Update / Aktualizacja** | `firmware/m5stack-touch-control-deck.update.bin` | `0x10000` | Zachowane tylko gdy układ partycji się nie zmienił i opcja **Erase device** jest wyłączona. |
| **Factory / Instalacja fabryczna** | `firmware/m5stack-touch-control-deck.factory.bin` | `0x0` | **Nadpisuje ustawienia**, w tym NVS. |

Manifesty ESP Web Tools: `firmware/manifest-update.json` i `firmware/manifest.json`. Instalator sprawdza zgodność wersji i dostępność obu plików `.bin`. Funkcji **Erase device** nie należy wybierać podczas aktualizacji, jeżeli zależy Ci na zapisanych ustawieniach.

ESP Web Tools may still offer **Erase device** in its dialog; do not select it for an update. Factory installation resets configuration. Requires a desktop Web Serial-capable browser, e.g. Chrome or Edge, and a USB cable that supports data transfer.

## Edycja strony / Customization

- `index.html`: struktura, linki, przyciski instalatora. Zachowaj `esp-web-install-button` i ścieżki do manifestów.
- `assets/site.css`: kolory, układ, animacje, responsywność, efekt kursora.
- `assets/site.js`: tłumaczenia PL/EN, odtwarzacz filmu, kontrola dostępności firmware, animacje.
- `assets/favicon.svg`: ikona strony.
- `../images/m5stack-touch-control-deck.jpg`: oryginalne zdjęcie publikowane przez istniejący skrypt budowania.
- `../docs/MACOS_FAST_SHORTCUTS.md`: instrukcja szybkiego otwierania URL na macOS.

**Uwaga:** Samodzielne publikowanie tylko HTML/CSS/JS bez uruchomienia kroku firmware spowoduje wyświetlenie informacji o niedostępnych obrazach. To celowe zabezpieczenie przed użyciem nieaktualnego firmware.

Project site: https://m5stack-touch-control-deck.lepczynski.it/
