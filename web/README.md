# Strona WWW i instalator firmware / Project website and firmware installer

Katalog `web/` zawiera statyczną, dwujęzyczną stronę projektu (PL/EN) oraz manifesty **ESP Web Tools**. **Nie jest potrzebny do kompilacji ani działania urządzenia.** Pliki `.bin` generowane są automatycznie podczas wdrożenia strony z kodu firmware w katalogu głównym repozytorium.

Instalator WWW ma dwa tryby:

- **Update / Aktualizacja**: wgrywa wyłącznie aplikację do `0x10000` i nie nadpisuje bezpośrednio partycji NVS. Aby zachować ustawienia, **nie wybieraj `Erase device`** w dialogu ESP Web Tools. Układ partycji musi pozostać taki sam.
- **Factory / Pierwsza instalacja**: zapisuje scalony obraz od `0x0`, nadpisując także obszar ustawień NVS. **Usuwa konfigurację** (hasła Wi-Fi, kafelki, strefy, profil OS). Wybierz tylko podczas pierwszej instalacji lub zamierzonego resetu.

ESP Web Tools może zaproponować opcję skasowania całej pamięci nawet w trybie aktualizacji, dlatego zachowanie ustawień nie jest gwarantowane, jeśli użytkownik zatwierdzi `Erase device`. Zalecamy zachowanie kopii własnej konfiguracji.

The `web/` folder contains the bilingual static website and ESP Web Tools manifests. **It is not needed to compile or run the device firmware.** The app-only update flashes at `0x10000`, preserving NVS only when **Erase device is NOT selected**, and the partition layout is unchanged. Factory install writes from `0x0` and **resets all settings**. Firmware binaries are built during deployment.

Project website: https://m5stack-touch-control-deck.lepczynski.it/

The product photograph comes from the repository `images/m5stack-touch-control-deck.jpg` and is copied to site assets during deployment. Deployment configuration in `wrangler.toml` is separate from the device build.
