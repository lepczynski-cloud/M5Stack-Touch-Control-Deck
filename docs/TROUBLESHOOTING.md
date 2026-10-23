# Rozwiązywanie problemów

## Skrypt nie wykrywa urządzenia

Uruchom:

```bat
tools\upload_windows.bat
```

Jeżeli port nie zostanie znaleziony:

1. użyj przewodu USB-C obsługującego dane,
2. przytrzymaj **RST** przez 2 do 3 sekund do zapalenia zielonej diody,
3. puść RST,
4. uruchom skrypt ponownie.

Port można wskazać jawnie:

```bat
tools\upload_windows.bat COM7
```

Porty opisane jako `Standardowy port szeregowy przez link Bluetooth` nie służą do wgrywania firmware.

## `ModuleNotFoundError: No module named 'intelhex'`

Skrypt powinien automatycznie zainstalować `intelhex==2.3.0`. Jeżeli instalacja została wcześniej przerwana:

1. usuń katalog `.pio-venv`,
2. uruchom ponownie `tools\upload_windows.bat`,
3. upewnij się, że komputer ma dostęp do Internetu i program antywirusowy nie blokuje lokalnego Pythona.

## Czarny ekran po wgraniu

Po uruchomieniu powinny pojawić się animacja startowa oraz etapy inicjalizacji. Jeżeli ekran pozostaje czarny, wejdź w tryb wgrywania przez RST i wgraj firmware ponownie.

Przycisk Power włącza i wyłącza urządzenie, ale nie zastępuje resetu.

## Nie można wrócić do wcześniej używanego komputera

Panel zapisuje jedno bieżące powiązanie BLE. Po sparowaniu z innym komputerem Windows na poprzednim komputerze może nadal przechowywać nieaktualny wpis.

Aby połączyć się ponownie:

1. usuń `M5 Control Deck` w **Ustawienia > Bluetooth i urządzenia** na komputerze,
2. na panelu przejdź do ekranu **Połączenia**,
3. wybierz **Nowe parowanie Bluetooth**,
4. dodaj urządzenie ponownie w Windows.

Samo usunięcie powiązania po stronie panelu nie usuwa wpisu zapisanego w Windows.

## Po parowaniu pojawia się wybór systemu

To zamierzone: po wykryciu nowego powiązania BLE wybierz **Windows**, **macOS** lub **Linux GNOME**. Wybór zapisuje się w pamięci i nie powinien powracać po zwykłym reconnect. Profil można zmienić na ekranie **Połączenia** albo przez `http://192.168.4.1` przy włączonym AP.

Jeśli zmieniłeś komputer i nowy system nie został wykryty, użyj **Nowe parowanie Bluetooth**, usuń stary wpis `M5 Control Deck` na poprzednim komputerze i sparuj urządzenie ponownie.

## macOS Tahoe: Spotlight nie otwiera się po dotknięciu kafelka

Firmware **v0.2.3 nie wyświetla już na M5Stacku pytania „Czy otwarto Spotlight?”**.

1. Na klawiaturze MacBooka naciśnij **`⌘ + Spacja`**. Sprawdź, czy Spotlight się otwiera.
2. Jeśli nie, przejdź do **Ustawienia systemowe → Klawiatura → Skróty klawiszowe → Spotlight** i sprawdź, czy włączono **„Pokaż wyszukiwanie Spotlight”** ze skrótem **`⌘ + Spacja`**.
3. Jeśli kombinacja działa na MacBooku, ale nie z M5Stacka, sprawdź mapowanie klawisza Command dla zewnętrznej klawiatury Bluetooth i konflikty z innymi launcherami.

Klawiatura BLE HID nie widzi aktywnego okna na Macu. Jeśli Spotlight nie otworzy się prawidłowo, niektóre wpisywane teksty nadal mogą trafić do aktywnej aplikacji. Zmiana w v0.2.3 nie jest automatycznym wykrywaniem fokusu.

## macOS: ChatGPT otwiera dokument zamiast strony WWW

Spotlight w macOS Tahoe może ustawić plik o nazwie „ChatGPT” wyżej niż wynik internetowy. Dlatego v0.2.3 **nie zatwierdza już adresu URL jako wyniku wyszukiwania Spotlight**.

Zamiast tego otwiera wybraną przeglądarkę jako aplikację (w Spotlight używa trybu **Aplikacje / Cmd+1**), a następnie przechodzi do paska adresu kombinacją `Cmd+L`, wpisuje URL i zatwierdza Enter.

1. Otwórz punkt dostępowy **M5-Control-Deck** i przejdź na `http://192.168.4.1`.
2. W sekcji **System komputera (Bluetooth HID)** wybierz **macOS** i przeglądarkę, której chcesz używać: **Google Chrome** (domyślnie), Safari, Firefox, Brave lub Microsoft Edge.
3. Zapisz ustawienia i poczekaj na restart M5Stacka.
4. Sprawdź kafelek ChatGPT przy otwartym TextEdit oraz gdy przeglądarka jest już uruchomiona.

**Ważne:** urządzenie **nie potrafi automatycznie odczytać domyślnej przeglądarki macOS** przez Bluetooth HID. Najlepiej wybrać w portalu tę samą przeglądarkę, która jest ustawiona jako domyślna w **Ustawienia systemowe → Biurko i Dock**. Zmiana wyboru przeglądarki jest zapisywana w NVS i nie powinna znikać po prawidłowej aktualizacji firmware przez WWW.

Jeśli aplikacja jest uruchamiana bardzo wolno, możesz potrzebować większego opóźnienia `MAC_BROWSER_ACTIVATION_MS` w `src/BleController.cpp`. W tej wersji jest to **2200 ms**. Działanie powinno być potwierdzone na konkretnym Macu: BLE HID nie zgłasza momentu przejęcia fokusu przez przeglądarkę.

## macOS: przycisk Pulpit nie pokazuje biurka

Poprzedni skrót `Cmd+F3` nie działał w testowanej konfiguracji macOS Tahoe. Od v0.2.3 M5Stack wysyła **`F11`** jako klawisz zewnętrznej klawiatury HID.

1. Na MacBooku sprawdź **`Fn + F11`**, które Apple dokumentuje jako „Pokaż biurko”.
2. Jeśli na M5Stacku F11 nie reaguje, otwórz **Ustawienia systemowe → Klawiatura → Skróty klawiszowe → Mission Control**.
3. Włącz **Pokaż biurko / Show Desktop** i przypisz do tej akcji **F11** (klawisz funkcyjny zewnętrznej klawiatury).

Nie każda konfiguracja interpretuje `F11` identycznie. Jeśli macOS lub inna aplikacja przechwytuje ten klawisz, przycisk może nie zadziałać. Wtedy trzeba sprawdzić konflikt skrótów na Macu.

## macOS: Finder, Monitor aktywności i pozostałe akcje

Zachowano działającą sekwencję uruchamiania Findera i Activity Monitor przez Spotlight oraz skróty `Cmd+Tab`, `Shift+Cmd+5` i `Ctrl+Cmd+Q`. Zmiany dotyczą wyłącznie URL, potwierdzenia Spotlight oraz przycisku Pulpit.

## Instalator WWW resetuje Wi-Fi i konfigurację po aktualizacji

**Tak działał poprzedni instalator.** Zapisywał scalony obraz firmware od `0x0` zawierający również obszar NVS. Kasowanie całej pamięci ESP Web Tools (`Erase device`) również usuwa ustawienia.

Od v0.2.2 wybierz na stronie **Update firmware / Aktualizuj firmware**, który zapisuje wyłącznie obraz aplikacji od `0x10000`, i **nie wybieraj `Erase device`** w dialogu ESP Web Tools. W ten sposób NVS może zostać zachowany, jeśli układ partycji się nie zmienił. **Factory install / Instalacja fabryczna** w dalszym ciągu celowo usuwa ustawienia. Nie da się odzyskać już skasowanej konfiguracji z samego firmware.

## Linux: niektóre kafelki nie działają

- Wybierz profil **Linux GNOME**. Inne środowiska, takie jak KDE, XFCE czy Cinnamon, nie mają gwarantowanych tych samych skrótów.
- Otwieranie adresów: GNOME `Alt+F2` i polecenie `xdg-open`.
- Monitor: wymagany `gnome-system-monitor`. Pliki: wymagany `nautilus`.
- Pulpit wywołuje **Activities**, a nie minimalizowanie wszystkich okien.
- Sprawdź lokalny układ klawiatury i skonfigurowane skróty.

## Komputer nie widzi klawiatury Bluetooth

- Otwórz ekran **Połączenia** i wybierz nowe parowanie.
- Usuń `M5 Control Deck` z listy urządzeń Windows.
- Dodaj urządzenie ponownie.
- Upewnij się, że Bluetooth w komputerze jest włączony.

## Kafelki aplikacji nie działają

- Sprawdź, czy ikona Bluetooth jest zielona.
- Upewnij się, że komputer jest odblokowany.
- Dla URL wymagany jest zgodny układ klawiatury; oryginalna konfiguracja Windows korzystała z **Polski programisty**. Na macOS i Linux sprawdź mapowanie klawiszy, zwłaszcza znaków `:`, `/`, `?` i `&`.
- Sprawdź, czy w **Połączeniach** jest wybrany właściwy system.
- Dla własnych adresów używaj standardowych znaków ASCII.

## Gest uruchamia kafelek zamiast zmienić ekran

Dostosuj w `src/UserInterface.cpp`:

- `SWIPE_THRESHOLD`,
- `SWIPE_DOMINANCE_MARGIN`,
- `TAP_MAX_MOVEMENT`,
- `TAP_MAX_DURATION_MS`.

Zwiększenie minimalnego dystansu gestu utrudnia przypadkowe przewijanie, natomiast zmniejszenie `TAP_MAX_MOVEMENT` ogranicza przypadkowe kliknięcia.

## Scroll Unit nie działa

- Podłącz go do Grove Port A.
- Sprawdź przewód Grove.
- Domyślny adres I2C to `0x40`.
- SDA to GPIO 2, a SCL to GPIO 1.
- Obrót i kliknięcie wysyłają polecenia dopiero po połączeniu Bluetooth.

Urządzenie może działać bez Scroll Unit. Sterowanie multimediami pozostaje dostępne na ekranie dotykowym.

## Zegar pokazuje `--:--`

- Włącz Wi-Fi.
- Zapisz prawidłowe dane sieci.
- Poczekaj na pierwszą synchronizację NTP.

Po pierwszej synchronizacji RTC przechowuje czas także podczas pracy offline.

## Portal nie otwiera się

1. Włącz AP na ekranie **Ustawienia**.
2. Połącz komputer z siecią `M5-Control-Deck`.
3. Otwórz `http://192.168.4.1`.
4. Użyj danych z `include/UserConfig.h` albo wcześniej zapisanej konfiguracji.

Domyślne dane:

```text
Login: admin
Hasło: admin123
```

## Błąd bibliotek lub niepełna kompilacja

Usuń katalog `.pio` i uruchom ponownie:

```bat
tools\build_windows.bat
```

Nie usuwaj katalogu źródłowego ani ustawień zapisanych w urządzeniu.

## Power nie otwiera menu

- Krótkie naciśnięcie jest rozpoznawane po puszczeniu przycisku.
- Dłuższe przytrzymanie rozpoczyna animowane wyłączenie.
- Przycisk RST służy do resetu i trybu wgrywania.

## Ekran nie przechodzi automatycznie w czuwanie

To prawidłowe zachowanie przy aktywnym połączeniu Bluetooth. Automatyczne odliczanie działa tylko wtedy, gdy BLE jest rozłączone. Sprawdź również ustawienie czuwania i czas bezczynności w portalu.

## Instalator WWW nie widzi urządzenia

- Otwórz [stronę instalatora](https://m5stack-touch-control-deck.lepczynski.it/) w zgodnej przeglądarce komputerowej obsługującej Web Serial (np. Chrome lub Edge), przez HTTPS.
- Użyj przewodu USB-C z transmisją danych, nie tylko do ładowania.
- Zamknij inne programy korzystające z portu COM.
- Wybierz port USB CoreS3 SE, nie wirtualny port Bluetooth.
- Jeśli port nie jest wykrywany, spróbuj sprzętowego trybu wgrywania przyciskiem RST i ponownie wybierz port.
- Instalator jest przeznaczony wyłącznie dla M5Stack CoreS3 SE.
- Jeśli na stronie nie ma gotowego firmware, skorzystaj z lokalnego skryptu Windows opisanego w [INSTALLATION.md](INSTALLATION.md).
