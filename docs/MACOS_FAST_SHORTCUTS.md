# macOS: szybsze otwieranie stron / Faster website launching

Na macOS dostępne są **dwa tryby** otwierania sześciu kafelków M5Stack Touch Control Deck. Oba korzystają ze standardowej klawiatury Bluetooth HID, bez instalowania dodatkowej aplikacji na komputerze.

| Tryb | Co robi M5Stack | Wymagana konfiguracja Maca |
| --- | --- | --- |
| **Standardowy** (domyślny) | `Cmd+Space` → Spotlight / Aplikacje → wybrana przeglądarka → `Cmd+L` → wpisanie URL → Enter | Sprawdź, czy `Cmd+Space` uruchamia Spotlight. W portalu M5Stack wybierz przeglądarkę. |
| **Szybki, natywne Skróty** | Wysyła tylko jeden skrót klawiszowy dla kafelka, np. `Ctrl+Option+Cmd+1`. **Nie wpisuje URL**. | Utwórz sześć skrótów w systemowej aplikacji **Skróty (Shortcuts)** na Macu. |

## Konfiguracja szybkiego trybu (macOS)

**Najpierw przygotuj Maca, dopiero potem włącz tryb na urządzeniu.** W przeciwnym razie dotknięcia kafelków wyślą kombinacje klawiszy bez przypisanych akcji.

1. Otwórz wbudowaną aplikację **Skróty (Shortcuts)** na Macu.
2. Utwórz nowy skrót przyciskiem `+`, nazwij go np. `Control Deck 1 - ChatGPT`.
3. Dodaj akcję **URL** z adresem `https://chatgpt.com/`, a zaraz po niej akcję **Otwórz adresy URL (Open URLs)**. Ta akcja otworzy stronę w domyślnej przeglądarce macOS.
4. Otwórz szczegóły skrótu (ikona `ⓘ` / Details), kliknij **Add Keyboard Shortcut / Dodaj skrót klawiszowy** i naciśnij `Control + Option (Alt) + Command (⌘) + 1`. W razie potrzeby włącz **Use as Quick Action / Użyj jako szybkiej czynności**, jeśli Twoja wersja macOS tego wymaga.
5. Uruchom skrót ręcznie, a następnie sprawdź kombinację na **fizycznej klawiaturze**. Testuj również wtedy, gdy na pierwszym planie jest inna aplikacja.
6. Powiel skrót pięć razy. Dla kolejnych kafelków zmień adres URL i ostatnią cyfrę skrótu klawiszowego według tabeli.
7. Sparuj M5Stack z Makiem przez Bluetooth. Włącz punkt dostępowy `M5-Control-Deck`, połącz się z nim i otwórz `http://192.168.4.1`. Zaloguj się danymi zapisanymi w konfiguracji urządzenia.
8. W sekcji **System komputera (Bluetooth HID)** wybierz **macOS**. W polu **Otwieranie stron na macOS** wybierz **Szybko: natywne Skróty macOS (1-6)**. Zapisz ustawienia. Urządzenie uruchomi się ponownie.
9. Testuj kafelki M5Stack przy otwartym TextEdit lub innej aplikacji, następnie przy przeglądarce na pierwszym planie. URL nie powinien być wpisywany przez symulowaną klawiaturę.

| Pozycja kafelka (czytanie z lewej do prawej, od góry) | Domyślna akcja | Globalna kombinacja |
| --- | --- | --- |
| 1 | ChatGPT · `https://chatgpt.com/` | `Control + Option + Command + 1` |
| 2 | Claude · `https://claude.ai/` | `Control + Option + Command + 2` |
| 3 | Gemini · `https://gemini.google.com/` | `Control + Option + Command + 3` |
| 4 | Canva · `https://www.canva.com/` | `Control + Option + Command + 4` |
| 5 | Admin Toolkit · `https://admintoolkit.lepczynski.it/` | `Control + Option + Command + 5` |
| 6 | Botland · `https://botland.com.pl/` | `Control + Option + Command + 6` |

> **Istotne:** W szybkim trybie adres przechowywany w **Skrótach na Macu** jest źródłem prawdy, a M5Stack wybiera numer akcji. Gdy zmienisz URL kafelka w portalu urządzenia, zmień również adres w odpowiadającym mu skrócie na Macu. Na innym Macu trzeba utworzyć analogiczne skróty. Nie musisz zmieniać kodu M5Stack.

### Jeśli nie działa

- Sprawdź globalną kombinację na fizycznej klawiaturze. macOS może przechwytywać zarezerwowane skróty lub wystąpił konflikt z innym programem. Kombinacje w firmware są stałe. W razie konfliktu zmień je w `src/BleController.cpp` **i** na Macu.
- W aplikacji Skróty sprawdź, czy masz jedną akcję **URL** z rzeczywistym adresem i akcję **Otwórz adresy URL**, a nie wyszukiwanie słów.
- Przy pierwszym uruchomieniu macOS może wyświetlić prośbę o zgodę na uruchomienie akcji. Zatwierdź ją ręcznie.
- Jeśli z Maca korzysta kilka kont, konfiguracja Skrótów jest specyficzna dla użytkownika i może się różnić.
- Aby przywrócić działanie bez przygotowanych skrótów, wybierz w portalu M5Stack tryb **Standardowo: Spotlight i przeglądarka**, zapisz i poczekaj na restart.
- Jeśli tryb standardowy otwiera przeglądarkę, lecz nie wpisuje URL, zwiększ stałą `MAC_BROWSER_ACTIVATION_MS` w `src/BleController.cpp` (domyślnie `1500` ms). Biblioteka BLE HID nie wie, kiedy aplikacja na Macu naprawdę przejęła fokus.

**Nie deklarujemy gwarantowanej liczby milisekund:** otwarcie strony nadal zależy od macOS, Skrótów i przeglądarki. Różnica polega na tym, że urządzenie nie odczekuje kilku sekund ani nie wysyła kolejnych znaków URL.

Oficjalna dokumentacja Apple: [Uruchamianie Skrótów za pomocą skrótu klawiaturowego](https://support.apple.com/guide/shortcuts-mac/run-a-shortcut-from-another-app-apd163eb9f95/mac).

---

## English: native macOS Shortcuts mode

The default mode opens Spotlight, finds the selected browser, focuses its address bar, then types the URL. This is now shorter, but **Bluetooth HID cannot detect whether the browser is ready**.

For the fastest and more focus-independent approach, create six shortcuts in the built-in macOS **Shortcuts** app:

1. Create a shortcut with the **URL** action set to `https://chatgpt.com/`, followed by **Open URLs**. Name it `Control Deck 1 - ChatGPT`.
2. Open shortcut **Details**, select **Add Keyboard Shortcut**, then press **Control + Option + Command + 1**. Verify this hotkey from another application. If necessary, enable **Use as Quick Action**.
3. Repeat for the remaining five tiles using the URLs and hotkeys `2` through `6` in the table above.
4. Open the device AP portal at `http://192.168.4.1`, select **macOS** and then **Fast: native macOS Shortcuts (1–6)**. Save and reboot.
5. If you change a tile's URL on the device, **also change its matching URL in Shortcuts**, because this mode sends only the corresponding keyboard shortcut, never the URL itself.

No third-party macOS application is required. If a global hotkey conflicts with another action, adjust both the firmware mapping and macOS. Return to **Standard: Spotlight and browser** in the portal if you prefer not to set up native shortcuts.
