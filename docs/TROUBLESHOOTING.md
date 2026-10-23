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

## macOS: tekst trafia do edytora zamiast uruchomić aplikację

W wersji **0.2.1** zdarzało się, że po naciśnięciu kafelka program wpisywał URL lub `Activity Monitor` do aktywnego dokumentu. Był to wynik wysyłania tekstu po `Cmd+Space`, mimo że Spotlight się nie otworzył. BLE HID nie dostarcza informacji o tym, jakie okno jest aktywne.

**W wersji 0.2.2:**

1. Na ekranie **Połączenia** wybierz **macOS** (ponowny wybór ponawia test).
2. Otwórz TextEdit na komputerze, żeby zauważyć ewentualny brak działania skrótu.
3. Naciśnij przycisk aplikacji, Finder albo Monitor aktywności na M5Stacku.
4. Panel wyśle **wyłącznie Cmd+Spacja**, a następnie zapyta: **Czy otwarto Spotlight?**
5. Jeżeli na Macu widać Spotlight, wybierz **TAK**, aby wysłać wybrane polecenie. Urządzenie zapamięta potwierdzenie i następne akcje będzie wykonywać bez tego pytania.
6. Jeżeli Spotlight się NIE pojawił, wybierz **NIE**. Wtedy urządzenie **nie wyśle żadnego tekstu do aktywnego edytora**.

Jeżeli Spotlight się nie otwiera, przetestuj **ręcznie** skrót `Cmd+Spacja` na klawiaturze Maca, a potem sprawdź **System Settings → Keyboard → Keyboard Shortcuts → Spotlight** oraz ewentualne konflikty z innym launcherem. W lokalnym portalu AP można zaznaczyć opcję wymuszenia ponownego testu.

Po poprawnym uruchomieniu Spotlight możesz sprawdzić ręcznie `Activity Monitor`, `Finder` i pełny URL. Wpisany URL powinien otworzyć się w domyślnej przeglądarce wybranej w **System Settings → Desktop & Dock**. Skrót **Pulpit** (`Cmd+F3`) wymaga poprawnej konfiguracji **Mission Control / Show Desktop**. Biblioteka HID może różnie działać z układem klawiatury Maca, zwłaszcza przy `:`, `/` i innych znakach specjalnych.

**Ograniczenie:** jednorazowe potwierdzenie nie zapewnia wykrywania fokusu przez BLE. Jeśli później przestawisz skrót lub launcher, uruchom test ponownie. Aby zawsze otwierać strony na Macu niezależnie od fokusu, potrzebna byłaby opcjonalna aplikacja pomocnicza. Poprzedni projekt ESP32 PC Control Deck korzystał z programu Python na hoście, więc jego uruchamianie stron nie jest bezpośrednio porównywalne z BLE HID.

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
