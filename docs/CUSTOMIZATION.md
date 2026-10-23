# Dostosowanie projektu

## Wartości domyślne

Najważniejsze ustawienia znajdują się w:

```text
include/UserConfig.h
```

Można tam zmienić:

- dane domowego Wi-Fi,
- nazwę i hasło punktu AP,
- login i hasło portalu,
- jasność ekranu,
- drugą strefę czasową,
- domyślny system hosta `HOST_OS` (Windows=0, macOS=1, Linux GNOME=2),
- nazwy i adresy sześciu kafelków,
- kierunek działania Scroll Unit,
- czas automatycznego czuwania.

Ustawienia zapisane przez portal trafiają do NVS i mają pierwszeństwo przed wartościami z kodu. Po zmianie wartości domyślnych użyj w portalu opcji **Ustawienia fabryczne**.

## Kafelki stron WWW

Domyślne nazwy i adresy znajdują się w tablicach `APP_LABELS` i `APP_URLS` w `include/UserConfig.h`.

Ikony są przypisane do pozycji kafelków w metodzie `drawAppsScreen()` w `src/UserInterface.cpp`. Ich kształty rysuje metoda `drawIcon()`.

Otwieranie adresów zależy od wybranego systemu: `Win+R` na Windows, Spotlight z pełnym URL (domyślna przeglądarka) na macOS, `Alt+F2` z `xdg-open` w GNOME. Dopuszczalne są adresy `http://` i `https://` z bezpiecznymi znakami ASCII; spacje i niestandardowe znaki koduj procentowo (np. `%20`).

## Zmiana skrótu dla systemu hosta

Klasa `BleController` w `src/BleController.cpp` wybiera odpowiednią kombinację na podstawie `hostOs_` i wartości `HostOS::Windows`, `HostOS::MacOS` albo `HostOS::LinuxGnome`.

1. Dodaj deklarację metody w `include/BleController.h`.
2. Dodaj implementację uwzględniającą wybrany `hostOs_` w `src/BleController.cpp`.
3. Przypisz metodę do kafelka w `handleWindowsTap()` (nazwa funkcji historyczna).
4. W `drawWindowsScreen()` ustaw etykietę dla każdego profilu oddzielnie.

Przykład skrótu Ustawienia Windows przez `Win + I`, dostępnego tylko gdy wybrano Windows:

```cpp
bool BleController::openWindowsSettings() {
    if (hostOs_ != HostOS::Windows) return false;
    return chord({KEY_LEFT_GUI, static_cast<uint8_t>('i')});
}
```

## Wybór systemu po nowym parowaniu

Firmware przechowuje wybrany `HostOS` i identyfikator ostatnio zaakceptowanego powiązania BLE (`pairedHostId`) w NVS (`src/ConfigManager.cpp`). Jeśli biblioteka NimBLE zgłosi nowe powiązanie, `src/UserInterface.cpp` pokazuje ekran wyboru systemu; zwykły reconnect nie wyświetla go ponownie. Profil można także wybrać na ekranie **Połączenia** lub w portalu WWW AP. Automatyczne rozpoznawanie systemu hosta nie jest dostępne w BLE HID.

**Linux oznacza GNOME/Ubuntu**, a nie dowolny menedżer pulpitu. `Alt+F2`, `gnome-system-monitor`, `nautilus` i `xdg-open` są zależne od instalacji. Na macOS otwieranie URL korzysta z Spotlight i domyślnej przeglądarki. Skrót `Cmd+F3` pokazuje pulpit, a Finder uruchamiany jest przez Spotlight. Skróty można zmieniać w ustawieniach macOS; urządzenie nie otrzymuje potwierdzenia przejęcia fokusu.

## Dodanie nowego ekranu

1. Dodaj wartość do `UserInterface::Screen` przed `Count`.
2. Dodaj ekran do metody `draw()`.
3. Dodaj metodę rysującą ekran.
4. Dodaj metodę obsługującą dotyk.

Wskaźnik stron automatycznie korzysta z wartości `Count`.

## Scroll Unit

Obsługa pokrętła znajduje się w `UserInterface::processEncoder()`.

Domyślnie:

- obrót wysyła głośniej lub ciszej,
- krótki nacisk wysyła Play/Pauza.

Kierunek obrotu można odwrócić w `include/UserConfig.h`:

```cpp
inline constexpr bool SCROLL_REVERSE_DIRECTION = true;
```

Jeżeli Scroll Unit nie jest podłączony, firmware uruchamia się normalnie i pozostawia wszystkie funkcje dotykowe.

## Czułość gestów

Progi znajdują się na początku `src/UserInterface.cpp`:

- `SWIPE_THRESHOLD`,
- `SWIPE_DOMINANCE_MARGIN`,
- `TAP_MAX_MOVEMENT`,
- `TAP_MAX_DURATION_MS`.

Ruch większy niż limit kliknięcia, ale za mały na pełny gest, jest ignorowany. Ogranicza to przypadkowe uruchamianie kafelków podczas przewijania.

## Napisy i polskie znaki

Wbudowane fonty M5GFX nie gwarantują pełnej obsługi znaków diakrytycznych. Z tego powodu krótkie napisy na ekranie używają wersji bez polskich znaków, natomiast portal i dokumentacja korzystają z pełnego języka polskiego.

## Animacje i zasilanie

Animacje znajdują się w `src/SystemAnimations.cpp` i są rysowane z prostych figur M5GFX, bez zewnętrznych bitmap.

Logika przycisku Power, czuwania i wybudzania znajduje się w `src/UserInterface.cpp`. Domyślne ustawienia są dostępne jako:

```cpp
STANDBY_ENABLED
STANDBY_TIMEOUT_MINUTES
STANDBY_COUNTDOWN_SECONDS
```

## Telemetria komputera

Profil klawiatury BLE może wysyłać klawisze i polecenia multimedialne, ale nie odczytuje CPU, RAM, GPU ani temperatur. Rozszerzenie projektu o telemetrię wymaga osobnego programu działającego na komputerze oraz dodatkowego kanału komunikacji, na przykład Wi-Fi albo USB.
