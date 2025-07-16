# MOOver_Merge - Pełna struktura katalogów i główne pliki

## Przegląd projektu
MOOver_Merge to repozytorium integrujące kod robotów MELKENS z systemem nawigacyjnym WB (Wasserbauer).

## Struktura katalogów głównych

```
MOOver_Merge/
├── .git/                    # Git repository metadata
├── WB/                      # Kod źródłowy oraz system firmware Wasserbauer
├── Melkens/                 # Kod źródłowy robota MELKENS
└── README.md                # Opis projektu (1.4KB)
```

## Szczegółowa struktura WB/

```
WB/
├── update                   # Plik aktualizacji (6.3MB)
├── rootfs.img              # System plików root (59MB)
├── linux.bin               # Kernel Linux (2.8MB)
├── config.sh               # Skrypt konfiguracyjny (773B)
└── bin/                    # Binaria i pliki wykonywalne
    ├── ButlerEngine        # Główny silnik aplikacji (20MB)
    ├── ButlerEvo           # Wersja Evolution aplikacji (21MB)
    ├── CANLogger           # Logger magistrali CAN (7.0MB)
    ├── PlcLogService       # Serwis logowania PLC (3.7MB)
    ├── canmsgmgr           # Menedżer wiadomości CAN (7.0MB)
    ├── ittiasql            # Baza danych SQL (1.6MB)
    ├── DDMap.cfg           # Konfiguracja mapowania (4.3KB)
    ├── ButlerEngine.xdd    # Definicja urządzenia CANopen (95KB)
    ├── settings            # Ustawienia systemu (332B)
    ├── *.wav               # Pliki dźwiękowe (alarm, beep, brum)
    ├── *.sh                # Skrypty Shell (autoupdate, backup, start, watch)
    ├── *.sql               # Skrypty SQL
    ├── canFlash/           # Narzędzia do flashowania CAN
    │   ├── canflash        # Narzędzie flashowania (4.4MB)
    │   ├── canFlash_GUI    # GUI flashowania (16MB)
    │   ├── *.hex           # Pliki firmware (magnetLineal, servo, steering, etc.)
    │   ├── *.xdd           # Definicje urządzeń CANopen
    │   └── *.xml           # Konfiguracje projektów
    ├── language/           # Pliki lokalizacji
    │   ├── qt_*.qm         # Pliki Qt w różnych językach
    │   ├── ButlerEngine_*.qm # Tłumaczenia ButlerEngine
    │   ├── ButlerEvo_GUI_*.qm # Tłumaczenia GUI
    │   └── WbQtGui_*.qm    # Tłumaczenia WbQtGui
    └── ftp/                # Katalog FTP
        └── PCSoftware/     # Oprogramowanie PC
```

## Szczegółowa struktura Melkens/

```
Melkens/
├── README.md               # Opis modułów MELKENS (294B)
├── setup_env.sh           # Skrypt setup dla Linux (161B)
├── setup_env.ps1          # Skrypt setup dla PowerShell (156B)
├── Melkens_PMB/           # Power Management Board - główna płytka sterująca
│   ├── Melkens_PMB.mc3    # Projekt MPLAB Code Configurator (3.4MB)
│   ├── README.md          # Dokumentacja PMB
│   ├── Routes.c           # Definicje tras (32KB, 927 linii)
│   ├── Makefile           # Konfiguracja budowania (3.3KB)
│   ├── main.c/.h          # Główne pliki aplikacji (5.0KB)
│   ├── CommonTypes.h      # Wspólne typy danych
│   ├── DataTypes.h        # Definicje typów (2.3KB)
│   ├── pmb_*.c/.h         # Moduły PMB:
│   │   ├── pmb_Functions  # Funkcje podstawowe
│   │   ├── pmb_Keyboard   # Obsługa klawiatury
│   │   ├── pmb_MotorManager # Zarządzanie silnikami (42KB)
│   │   ├── pmb_RouteManager # Zarządzanie trasami (53KB)
│   │   ├── pmb_Scheduler  # Scheduler zadań
│   │   ├── pmb_System     # System operacyjny
│   │   ├── pmb_Display    # Obsługa wyświetlacza (35KB)
│   │   └── pmb_CAN        # Komunikacja CAN
│   ├── DiagnosticsHandler.c/.h # Obsługa diagnostyki
│   ├── DriveIndicator.c/.h # Wskaźniki jazdy
│   ├── RoutesDataTypes.h  # Typy danych tras
│   ├── setup_symlinks.*   # Skrypty linków symbolicznych
│   ├── AnalogHandler/     # Obsługa sygnałów analogowych
│   ├── BatteryManager/    # Zarządzanie baterią
│   ├── CANHandler/        # Obsługa magistrali CAN
│   │   ├── CANHandler.c   # Implementacja CAN
│   │   └── CanHandler.h   # Nagłówki CAN
│   ├── DmaController/     # Kontroler DMA
│   ├── IMUHandler/        # Obsługa IMU (Inertial Measurement Unit)
│   │   ├── IMUHandler.c   # Implementacja IMU (18KB, 646 linii)
│   │   └── IMUHandler.h   # Nagłówki IMU (4.7KB)
│   ├── LedHandler/        # Obsługa LED
│   ├── TimeManager/       # Zarządzanie czasem
│   ├── Tools/             # Narzędzia pomocnicze
│   ├── mcc_generated_files/ # Pliki generowane przez MCC
│   └── nbproject/         # Konfiguracja NetBeans
├── Melkens_IMU/           # Inertial Measurement Unit - moduł IMU
│   ├── Melkens_IMU.ioc    # Konfiguracja STM32CubeMX (17KB)
│   ├── README.md          # Dokumentacja IMU (42B)
│   ├── STM32G473RCTX_*.ld # Skrypty linkera STM32
│   ├── setup_symlinks.*   # Skrypty konfiguracji
│   ├── .cproject/.project # Konfiguracje Eclipse
│   ├── .mxproject         # Projekt MX
│   ├── Core/              # Kod główny STM32
│   │   ├── Src/           # Pliki źródłowe
│   │   ├── Inc/           # Pliki nagłówkowe
│   │   └── Startup/       # Pliki startowe
│   └── Drivers/           # Sterowniki HAL STM32
├── Melkens_Connectivity/  # Moduł łączności (Arduino/ESP32)
│   ├── Melkens_Connectivity.ino # Główny plik Arduino (9.8KB, 352 linie)
│   ├── README.md          # Dokumentacja connectivity (2.7KB)
│   ├── library.properties # Właściwości biblioteki Arduino
│   ├── keywords.txt       # Słowa kluczowe IDE
│   ├── setup_symlinks.*   # Skrypty konfiguracji
│   ├── data/              # Dane statyczne
│   └── src/               # Kod źródłowy bibliotek
│       ├── Settings.h     # Ustawienia globalne
│       ├── WebHandler/    # Obsługa serwera WWW
│       ├── WebPage/       # Strony internetowe
│       ├── BleSerial/     # Komunikacja Bluetooth LE
│       ├── ImuCommunication/ # Komunikacja z IMU
│       └── MqttNode/      # Węzeł MQTT
└── Melkens_Lib/           # Biblioteki wspólne
    ├── CRC16/             # Biblioteka CRC16
    │   ├── CRC16.c        # Implementacja CRC16 (2.5KB)
    │   └── CRC16.h        # Nagłówki CRC16
    └── Types/             # Typy danych
        └── MessageTypes.h # Typy wiadomości (1.9KB)
```

## Główne komponenty systemu

### WB (Wasserbauer)
- **ButlerEngine/ButlerEvo**: Główne aplikacje nawigacyjne
- **CANLogger/canmsgmgr**: Obsługa komunikacji CAN
- **canFlash**: Narzędzia do aktualizacji firmware
- **rootfs.img/linux.bin**: Kompletny system Linux
- **Wielojęzyczność**: Wsparcie dla ~15 języków

### MELKENS
- **PMB (Power Management Board)**: Główna płytka sterująca z obsługą silników, tras, CAN
- **IMU**: Moduł inercyjny na STM32G473
- **Connectivity**: Moduł łączności z WiFi/BLE na ESP32
- **Shared Libraries**: Wspólne biblioteki (CRC16, typy komunikatów)

## Kluczowe pliki konfiguracyjne
- `WB/config.sh` - Konfiguracja systemu WB
- `WB/bin/DDMap.cfg` - Mapowanie urządzeń
- `WB/bin/settings` - Ustawienia aplikacji
- `Melkens/*/setup_symlinks.*` - Skrypty konfiguracji linków
- `Melkens/Melkens_IMU/Melkens_IMU.ioc` - Konfiguracja STM32
- `Melkens/Melkens_Connectivity/library.properties` - Biblioteka Arduino

## Statystyki
- **Całkowity rozmiar**: ~150MB (głównie binaria WB)
- **Języki programowania**: C, C++, Shell, SQL, Arduino/C++
- **Platformy**: Linux (WB), PIC (PMB), STM32 (IMU), ESP32 (Connectivity)
- **Protokoły komunikacji**: CAN, UART, SPI, I2C, WiFi, BLE, MQTT