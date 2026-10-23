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

## macOS: skróty lub otwieranie URL nie działają

- Na ekranie Połączenia wybierz **macOS**, nie Windows.
- Do otwierania adresów firmware korzysta z **Cmd+Space → Safari → Cmd+L**; Spotlight i Safari muszą być dostępne.
- macOS może mieć inne skróty Spotlight, inne układy klawiatury i ustawienia dostępności. Sprawdź standardowe skróty w Ustawieniach systemowych.
- **Opcja Pliki otwiera okno wyszukiwania Findera**, a widok Pulpit odpowiada Mission Control.
- Wsparcie macOS wymaga dodatkowych testów na fizycznym komputerze; biblioteka BLE dla macOS ma ograniczenia zgodności.

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
