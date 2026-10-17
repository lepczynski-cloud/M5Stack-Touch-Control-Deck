# Instalacja i wgrywanie w Windows

## Wymagania

- Windows 10 lub Windows 11,
- Python 3.9 lub nowszy,
- przewód USB-C obsługujący transmisję danych,
- dostęp do Internetu podczas pierwszego budowania.

## Najprostsze wgranie

Podłącz CoreS3 SE i uruchom z głównego katalogu projektu:

```bat
tools\upload_windows.bat
```

Skrypt:

1. sprawdza pliki projektu,
2. tworzy lokalne środowisko `.pio-venv`,
3. instaluje PlatformIO,
4. uzupełnia brakującą zależność `intelhex`,
5. kompiluje firmware,
6. wyszukuje port USB Espressif,
7. pomija wirtualne porty COM utworzone przez Bluetooth,
8. wgrywa firmware na zweryfikowany port.

Przy pierwszym uruchomieniu pobieranie narzędzi może potrwać kilka minut.

## Ręczne wskazanie portu

Gdy komputer pokazuje kilka portów, podaj właściwy port CoreS3 SE:

```bat
tools\upload_windows.bat COM7
```

Nie wybieraj portu opisanego jako `Standardowy port szeregowy przez link Bluetooth`.

## Wejście w tryb wgrywania

Jeżeli skrypt nie widzi CoreS3 SE:

1. pozostaw urządzenie podłączone przez USB-C,
2. przytrzymaj przycisk **RST** przez 2 do 3 sekund,
3. puść RST po zapaleniu zielonej diody,
4. uruchom ponownie `tools\upload_windows.bat`.

Przycisk Power nie zastępuje przycisku RST.

## Sama kompilacja

```bat
tools\build_windows.bat
```

## Monitor portu szeregowego

Automatyczne wykrycie portu:

```bat
tools\monitor_windows.bat
```

Ręczne wskazanie portu:

```bat
tools\monitor_windows.bat COM7
```

Prędkość monitora wynosi 115200 bit/s. Monitor zakończ skrótem `Ctrl + C`.

## CLion lub VS Code z PlatformIO

Repozytorium jest standardowym projektem PlatformIO. Otwórz katalog projektu i wybierz środowisko `cores3_se`. Cel `Upload` kompiluje i wgrywa firmware.

Plik `tools/ensure_platformio_deps.py` jest uruchamiany przed budowaniem i sprawdza zależność `intelhex`, dlatego ten sam mechanizm działa również przy kompilacji bez skryptu BAT.

## Katalogi robocze

PlatformIO i środowiska programistyczne mogą utworzyć między innymi:

```text
.pio
.pio-venv
.idea
.vscode
cmake-build-*
```

Są ignorowane przez Git i nie są częścią gotowego ZIP-a.
