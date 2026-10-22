# Project website / Strona projektu

**PL:** Ten katalog zawiera dwujęzyczną, statyczną stronę M5Stack Touch Control Deck oraz instalator firmware uruchamiany w przeglądarce. **Nie jest potrzebny do budowy, lokalnego kompilowania ani działania samego urządzenia.** Do modyfikowania firmware służą katalogi `include/` i `src/`, plik `platformio.ini` oraz skrypty `tools/`.

**EN:** This directory contains the bilingual static M5Stack Touch Control Deck website and the browser-based firmware installer. **It is not needed to assemble the device, compile its firmware locally, or run it.** Device firmware lives in `include/` and `src/`, with `platformio.ini` and the `tools/` scripts.

Strona / Website: [M5Stack-Touch-Control-Deck.lepczynski.it](https://m5stack-touch-control-deck.lepczynski.it/)

Zawartość / Contents:
- `index.html` - strona WWW / static website;
- `assets/` - style, tłumaczenia, grafika / styles, translations and media;
- `firmware/` - manifest i gotowy obraz firmware generowany podczas publikacji / install manifest and release firmware image generated during deployment.

Źródłowe pliki firmware pozostają w pozostałej części repozytorium. Strona wgrywa już skompilowany obraz, **nie kompiluje kodu na komputerze odwiedzającego**.

The product photograph is sourced from `images/m5stack-touch-control-deck.jpg` in the main repository and copied to website assets during deployment.

`wrangler.toml` w katalogu głównym definiuje nazwę i statyczne zasoby opcjonalnej strony WWW. Nie jest używany podczas kompilacji programu dla M5Stacka.

The root `wrangler.toml` defines the optional website deployment name and assets. It has no role in building firmware for the device.
