# M5Stack Touch Control Deck
**Instalator WWW i strona projektu:** [m5stack-deck.lepczynski.it](https://m5stack-deck.lepczynski.it/) (PL/EN; automatyczna kompilacja firmware w Cloudflare Pages, instrukcja: [web/README.md](web/README.md)).

Dotykowy i bezprzewodowy panel sterowania dla komputerów z Windows 10 i Windows 11. Projekt wykorzystuje M5Stack CoreS3 SE jako klawiaturę Bluetooth Low Energy oraz kontroler multimediów, dlatego podstawowe funkcje działają bez instalowania dodatkowego programu na komputerze.

Projekt rozwija pomysł z repozytorium [ESP32 PC Control Deck](https://github.com/lepczynski-cloud/ESP32-PC-Control-Deck). Wcześniejszą konstrukcję można zobaczyć na [filmie w YouTube](https://www.youtube.com/watch?v=16eqHU4j9Fk).

Dokumentacja po angielsku: [README_EN.md](README_EN.md).

**Film pokazujący budowę nowej wersji:** [M5Stack Touch Control Deck na YouTube](https://www.youtube.com/watch?v=Jz1xCBi-Q1o).

## Najważniejsze funkcje

- sześć ekranów przewijanych pionowym gestem,
- dwa zegary: Polska oraz konfigurowana druga strefa, domyślnie HKT,
- opcjonalna synchronizacja NTP przez Wi-Fi i praca offline z RTC,
- sześć konfigurowanych kafelków stron WWW,
- sterowanie multimediami z ekranu dotykowego,
- opcjonalny Scroll Unit: obrót reguluje głośność, a kliknięcie wysyła Play/Pauza,
- najczęściej używane skróty Windows 10/11,
- lokalny portal konfiguracyjny dostępny przez własny punkt AP,
- ikony stanu Wi-Fi, Bluetooth i akumulatora,
- animowany start, czuwanie, wybudzanie i wyłączanie,
- menu zasilania pod krótkim naciśnięciem Power,
- automatyczna propozycja czuwania tylko wtedy, gdy Bluetooth nie jest połączony.

## Zgodność

Pierwsze publiczne wydanie obsługuje wyłącznie Windows 10 i Windows 11. Otwieranie stron korzysta z `Win + R`, a ekran systemowy wysyła skróty charakterystyczne dla Windows.

Urządzenie działa jako standardowa klawiatura BLE HID. Ten profil nie dostarcza wiarygodnej informacji o systemie operacyjnym komputera, dlatego pierwsze wydanie korzysta z jednego, jednoznacznego zestawu skrótów dla Windows.

## Sprzęt użyty w tym projekcie

### Element wymagany

1. [M5Stack CoreS3 SE, K128-SE](https://botland.com.pl/seria-core/25332-m5stack-cores3-se-modul-deweloperski-esp32-s3-m5stack-k128-se-6972934175304.html)
2. przewód USB-C obsługujący transmisję danych, potrzebny do pierwszego wgrania firmware

### Elementy opcjonalne

- [M5Stack Battery Module 13.2, 1500 mAh, M120](https://botland.com.pl/seria-core/21459-m5stack-battery-module-132-nakladka-z-akumulatorem-1500mah-do-modulow-deweloperskich-m5stack-core-m5stack-m120-6972934173546.html)
- [M5Stack Scroll Unit, U186](https://botland.com.pl/moduly-unit-moduly-rozszerzen/25428-scroll-enkoder-obrotowy-z-przyciskiem-modul-rozszerzen-unit-m5stack-u186-6972934175441.html)

Bez Battery Module panel może pracować z zasilania USB-C. Bez Scroll Unit wszystkie ekrany dotykowe nadal działają, a funkcje multimedialne pozostają dostępne na ekranie.

Scroll Unit podłącz do niebieskiego portu Grove A. Firmware używa adresu I2C `0x40`, SDA GPIO 2 i SCL GPIO 1 przez magistralę `M5.Ex_I2C`.

## Obudowa 3D użyta w projekcie

W swoim egzemplarzu użyłem gotowego modelu [M5Stack Core 2 case 45° angle w/Unit holder](https://makerworld.com/en/models/3128546-m5stack-core-2-case-45deg-angle-w-unit-holder#profileId-3530280) przygotowanego przez użytkownika Gigi w serwisie MakerWorld. Nie jest to mój projekt i pliki obudowy nie są częścią tego repozytorium.

Model został przygotowany z myślą o serii M5Stack Core i bocznym module Unit. W moim zestawie z CoreS3 SE, Battery Module 13.2 i Scroll Unit pasuje bardzo dobrze. Obudowa nie jest wymagana, można użyć innego modelu albo pozostawić urządzenie bez obudowy.

## Ekrany

### Zegar

Pokazuje czas w Polsce, drugi zegar oraz datę. Niebieskie tło oznacza aktywne Wi-Fi i aktualny czas po synchronizacji, a szare tło pracę offline z RTC.

### Aplikacje

Układ 2 x 3:

| ChatGPT | Claude | Gemini |
|---|---|---|
| Canva | Admin Toolkit | Botland |

Dotknięcie kafelka wysyła `Win + R`, wpisuje adres i naciska Enter. Nazwy oraz adresy można zmienić w portalu WWW lub w `include/UserConfig.h`.

### Multimedia

- poprzedni utwór,
- Play/Pauza,
- następny utwór,
- wyciszenie,
- ciszej,
- głośniej.

Jeżeli podłączono Scroll Unit, obrót zmienia głośność, a kliknięcie steruje Play/Pauza.

### Windows

- Menedżer zadań: `Ctrl + Shift + Esc`,
- blokada komputera: `Win + L`,
- narzędzie wycinania: `Win + Shift + S`,
- pokaż pulpit: `Win + D`,
- Eksplorator plików: `Win + E`,
- przełącz okno: `Alt + Tab`.

Kafelek **Wycinek** uruchamia narzędzie pozwalające zaznaczyć fragment ekranu. Nie wykonuje automatycznie pełnego zrzutu.

### Ustawienia

- jasność,
- włączenie lub wyłączenie domowego Wi-Fi,
- włączenie lub wyłączenie punktu AP,
- wybór drugiej strefy czasowej.

### Połączenia

- status Bluetooth,
- uruchomienie nowego parowania,
- dane lokalnego portalu,
- stan Scroll Unit,
- wersja firmware.

## Szybkie wgranie w Windows

Wymagany jest Python 3.9 lub nowszy oraz dostęp do Internetu podczas pierwszego uruchomienia.

1. Podłącz CoreS3 SE przewodem USB-C obsługującym dane.
2. Uruchom:

```bat
tools\upload_windows.bat
```

Skrypt tworzy lokalne środowisko `.pio-venv`, instaluje PlatformIO, kompiluje firmware, odrzuca wirtualne porty COM Bluetooth i wgrywa program na port USB Espressif.

Port można wskazać ręcznie:

```bat
tools\upload_windows.bat COM7
```

Gdy CoreS3 SE nie jest wykrywany, przytrzymaj **RST** przez 2 do 3 sekund do zapalenia zielonej diody, puść przycisk i uruchom skrypt ponownie.

Pełna instrukcja: [docs/INSTALLATION.md](docs/INSTALLATION.md).

## Pierwsze parowanie Bluetooth

1. W Windows otwórz **Ustawienia > Bluetooth i urządzenia > Dodaj urządzenie > Bluetooth**.
2. Wybierz **M5 Control Deck**.
3. Po połączeniu ikona Bluetooth na pasku urządzenia zmieni kolor na zielony.

### Zmiana komputera

Panel przechowuje tylko jedno aktywne powiązanie BLE.

1. Na ekranie **Połączenia** wybierz **Nowe parowanie Bluetooth**.
2. Potwierdź usunięcie bieżącego powiązania.
3. Na poprzednim komputerze usuń `M5 Control Deck` z listy urządzeń Bluetooth.
4. Sparuj panel z nowym komputerem.

Windows może zachować stare powiązanie po stronie komputera. Gdy później wracasz do wcześniej używanego komputera, usuń z niego istniejący wpis `M5 Control Deck`, ponownie uruchom nowe parowanie na panelu i dodaj urządzenie od początku.

## Portal konfiguracji

Jeżeli nie zapisano danych domowego Wi-Fi, urządzenie uruchomi punkt dostępowy:

```text
Sieć: M5-Control-Deck
Hasło AP: m5control
Adres: http://192.168.4.1
Login: admin
Hasło panelu: admin123
```

AP można również włączyć ręcznie na ekranie **Ustawienia**, także podczas połączenia z domowym Wi-Fi. Portal umożliwia zmianę sieci Wi-Fi, jasności, drugiej strefy czasowej, czasu bezczynności, danych AP, danych logowania oraz nazw i adresów sześciu kafelków.

Po pierwszym uruchomieniu warto zmienić domyślne hasło AP i hasło panelu. Portal działa lokalnie przez HTTP, dlatego po zakończeniu konfiguracji najlepiej wyłączyć punkt AP.

## Konfiguracja w kodzie

Najważniejsze wartości znajdują się w:

```text
include/UserConfig.h
```

Można tam ustawić dane Wi-Fi, parametry punktu AP, jasność, drugą strefę czasową, nazwy i adresy kafelków, kierunek pokrętła oraz domyślne zachowanie czuwania.

Zmiany zapisane wcześniej w NVS mają pierwszeństwo przed wartościami domyślnymi. Po edycji ustawień domyślnych użyj opcji **Ustawienia fabryczne** w portalu.

Szczegóły modyfikacji: [docs/CUSTOMIZATION.md](docs/CUSTOMIZATION.md).

## Zasilanie i czuwanie

- Krótkie naciśnięcie **Power** otwiera menu z opcjami **Czuwanie**, **Wyłącz** i **Anuluj**.
- Dłuższe przytrzymanie Power rozpoczyna animowane wyłączenie.
- Czuwanie wyłącza tylko ekran. Bluetooth, Wi-Fi, zegar i portal pozostają aktywne.
- Dotknięcie ekranu, użycie Scroll Unit albo Power wybudza panel.
- Przy aktywnym połączeniu Bluetooth urządzenie nie proponuje automatycznego czuwania.

## Ograniczenia

- Obsługiwane są Windows 10 i Windows 11.
- Projekt zakłada układ klawiatury **Polski programisty**.
- Komputer musi być odblokowany, aby `Win + R` mógł otworzyć stronę.
- BLE HID nie udostępnia CPU, RAM, GPU ani temperatur. Takie dane wymagają osobnego programu działającego w Windows.
- Urządzenie przechowuje jedno bieżące powiązanie Bluetooth.

## Struktura repozytorium

```text
.
├── include/                 konfiguracja i deklaracje modułów
├── src/                     kod firmware
├── docs/                    instalacja, modyfikacje i diagnostyka
├── tools/                   skrypty dla Windows
├── README_EN.md             pełna instrukcja po angielsku
└── platformio.ini           konfiguracja PlatformIO
```

Kod firmware pozostaje podzielony na niewielkie moduły, ponieważ ułatwia to modyfikowanie interfejsu, Bluetooth, sieci i zasilania bez naruszania pozostałych części projektu.

## Licencja

Kod jest dostępny na licencji MIT. Zobacz [LICENSE](LICENSE).
