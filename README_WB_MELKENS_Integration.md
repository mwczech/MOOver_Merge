# WB-MELKENS Integration Project

Kompletna implementacja integracji systemów Wasserbauer (WB) z platformą MELKENS, zawierająca analizę różnic w nawigacji, emulację zachowań WB oraz symulator porównawczy algorytmów.

## 📋 Spis treści

1. [Przegląd projektu](#przegląd-projektu)
2. [Struktura projektu](#struktura-projektu) 
3. [Analiza różnic nawigacji](#analiza-różnic-nawigacji)
4. [Emulator WB w MELKENS](#emulator-wb-w-melkens)
5. [Symulator porównawczy](#symulator-porównawczy)
6. [Instrukcje instalacji](#instrukcje-instalacji)
7. [Instrukcje użytkowania](#instrukcje-użytkowania)
8. [Dokumentacja API](#dokumentacja-api)
9. [Testowanie](#testowanie)
10. [Rozwiązywanie problemów](#rozwiązywanie-problemów)

---

## 🎯 Przegląd projektu

### Cel główny
Utworzenie warstwy kompatybilności umożliwiającej robotom MELKENS współpracę z systemem nawigacji Wasserbauer (WB) przy zachowaniu pełnej funkcjonalności obu systemów.

### Kluczowe osiągnięcia
- ✅ **Analiza porównawcza** systemów nawigacji MELKENS vs WB
- ✅ **Implementacja emulacji WB** w środowisku MELKENS
- ✅ **Symulator wizualny** do porównania algorytmów
- ✅ **Warstwa kompatybilności** CANopen z WB
- ✅ **Dokumentacja techniczna** i instrukcje integracji

---

## 📁 Struktura projektu

```
MOOver_Merge/
├── Navigation_Differences_Analysis.md          # Analiza różnic nawigacji
├── Robot_Navigation_Simulator.py               # Symulator porównawczy Python
├── Melkens/Melkens_PMB/CANHandler/
│   ├── WB_Navigation_Emulator.h                # Header emulatora WB
│   ├── WB_Navigation_Emulator.c                # Implementacja emulatora WB
│   ├── WB_CanOpen.h                           # Warstwa CANopen WB
│   ├── WB_CanOpen.c                           # Implementacja CANopen
│   └── WB_Config.h                            # Konfiguracja WB
├── structure_overview.md                       # Przegląd struktury repozytorium
├── completeness_analysis.md                    # Analiza kompletności projektów
└── README_WB_Integration.md                    # Dokumentacja integracji
```

---

## 🧭 Analiza różnic nawigacji

### Kluczowe różnice architektoniczne

| Aspekt | MELKENS | WB (Wasserbauer) |
|--------|---------|-------------------|
| **Architektura** | Step-based routes | World coordinate system |
| **Pozycjonowanie** | Relative positioning | Absolute coordinates |
| **Planowanie trasy** | Predefiniowane kroki | Dynamiczne współrzędne |
| **Korekcja magnetyczna** | Predefined corrections | Dynamic field strength |
| **Reakcja na błędy** | Restart sekwencji | Przeliczenie trasy |

### Szczegóły implementacji

#### MELKENS - Navigation Step-based
```c
typedef struct RouteStep_t {
    uint8_t OperationType;     // NORM, TU_L, TU_R, L_90, R_90
    uint16_t dX, dY;          // Relative distance changes
    uint16_t RightSpeed, LeftSpeed;
    float Angle;
    float MagnetCorrection;   // Predefined correction values
} RouteStep;
```

#### WB - World Coordinate Navigation  
```c
typedef struct {
    float posX, posY;         // Absolute world coordinates
    uint16_t direction;       // World direction
    uint16_t trommelSpeed;
    uint16_t butlerSpeed;
    float fieldStrengthThreshold;  // Dynamic magnetic threshold
} WB_TrackPos_t;
```

---

## 🔧 Emulator WB w MELKENS

### Funkcjonalność
Emulator WB umożliwia robotom MELKENS:
- Nawigację w stylu WB (world coordinates)
- Dynamiczną korekcję magnetyczną  
- Obsługę komend Butler Control
- Emulację zachowań bay approach/feeding
- Kompatybilność z protokołem CANopen WB

### Kluczowe komponenty

#### 1. Navigation Context
```c
typedef struct {
    WB_WorldPosition_t currentPos;
    WB_WorldPosition_t targetPos;
    WB_NavigationState_t state;
    
    // Path planning
    float pathDistance;
    float remainingDistance;
    float crossTrackError;
    float headingError;
    
    // Magnetic positioning
    uint32_t lastReferenceId;
    float magneticFieldStrength;
    bool magneticPositionValid;
} WB_NavigationContext_t;
```

#### 2. Magnetic Field Simulation
```c
typedef struct {
    float strength;          // Field strength (0.0 - 100.0)
    float position;          // Position relative to center (-15.0 to +15.0)
    bool detected;           // Magnet detected flag
    uint32_t referenceId;    // Reference position ID
} WB_MagneticField_t;
```

### API Functions

#### Podstawowe funkcje nawigacji
```c
void WB_NavEmulator_Init(void);
void WB_NavEmulator_Update(void);
bool WB_NavEmulator_SetTarget(float x, float y, float heading);
bool WB_NavEmulator_NavigateToTrack(uint32_t trackId);
bool WB_NavEmulator_ApproachBay(uint32_t bayId);
```

#### Symulacja detekcji magnetów
```c
void WB_NavEmulator_SimulateMagnetDetection(MagnetName detectedMagnet);
void WB_NavEmulator_ProcessMagneticField(float fieldStrength, float position);
float WB_NavEmulator_CalculateMagneticCorrection(float fieldStrength, float targetPosition);
```

#### Integracja z MELKENS
```c
bool WB_NavEmulator_ConvertMelkensRoute(const RouteStep* melkensStep);
void WB_NavEmulator_ApplyToMelkensMotors(float speed, float steering);
void WB_NavEmulator_SyncWithMelkens(void);
```

---

## 🎮 Symulator porównawczy

### Opis
Wizualny symulator Python umożliwiający porównanie algorytmów nawigacji MELKENS vs WB w czasie rzeczywistym.

### Funkcje symulatora
- **Wizualizacja robotów** w środowisku 2D
- **Porównanie ścieżek** obu algorytmów
- **Metryki wydajności** w czasie rzeczywistym
- **Środowisko testowe** z torami i magnetami
- **Eksport wyników** do analizy

### Kluczowe klasy

#### Robot Model
```python
class Robot:
    def __init__(self, initial_pos: Position):
        self.position = initial_pos
        self.velocity = 0.0
        self.steering_angle = 0.0
        self.state = RobotState.IDLE
        
        # Performance metrics
        self.distance_traveled = 0.0
        self.energy_consumed = 0.0
        self.magnet_detections = 0
```

#### MELKENS Navigator
```python
class MelkensNavigator:
    def __init__(self, robot: Robot):
        self.robot = robot
        self.route_steps = []
        self.current_step_index = 0
        self.magnet_positions = list(range(-15, 16))
        self.magnet_corrections = {i: i * 2.17 for i in self.magnet_positions}
```

#### WB Navigator  
```python
class WBNavigator:
    def __init__(self, robot: Robot):
        self.robot = robot
        self.target_position = None
        self.cross_track_error = 0.0
        self.heading_error = 0.0
        
        # PID controller parameters
        self.kp_cross = 2.0
        self.kp_heading = 1.0
```

---

## 🚀 Instrukcje instalacji

### Wymagania systemowe

#### Środowisko MELKENS (C/Embedded)
- MPLAB X IDE v5.40+
- STM32 Cube IDE v1.7+
- Arduino IDE v1.8.13+
- XC16 Compiler v1.70+

#### Symulator Python
- Python 3.8+
- NumPy 1.21+
- Matplotlib 3.4+
- Dataclasses (Python 3.7+)

### Instalacja środowiska MELKENS

1. **Klonowanie repozytorium**
```bash
git clone <repository-url>
cd MOOver_Merge
```

2. **Konfiguracja środowiska MELKENS**
```bash
cd Melkens
chmod +x setup_env.sh
./setup_env.sh
```

3. **Kompilacja projektów**
```bash
# PMB (MPLAB X)
cd Melkens_PMB
# Otwórz w MPLAB X IDE i skompiluj

# IMU (STM32 Cube IDE)  
cd Melkens_IMU
# Otwórz w STM32 Cube IDE i skompiluj

# Connectivity (Arduino IDE)
cd Melkens_Connectivity
# Otwórz w Arduino IDE i wgraj na ESP32
```

### Instalacja symulatora Python

1. **Instalacja zależności**
```bash
pip install numpy matplotlib dataclasses
# lub używając conda:
conda install numpy matplotlib
```

2. **Uruchomienie symulatora**
```bash
python Robot_Navigation_Simulator.py
```

---

## 📖 Instrukcje użytkowania

### 1. Integracja z istniejącym kodem MELKENS

#### Dodanie do projektu PMB
```c
#include "CANHandler/WB_Navigation_Emulator.h"

// W main() lub init function
void main(void) {
    // ... existing MELKENS init ...
    
    // Initialize WB emulator
    WB_NavEmulator_Init();
    
    // Main loop
    while(1) {
        // ... existing MELKENS code ...
        
        // Update WB emulator
        WB_NavEmulator_Update();
        
        // Check for magnet detection
        if (magnet_detected) {
            WB_NavEmulator_SimulateMagnetDetection(detected_magnet);
        }
    }
}
```

#### Konwersja tras MELKENS na nawigację WB
```c
// Convert existing MELKENS route to WB navigation
RouteStep melkens_step = {
    .OperationType = NORM,
    .dX = 500,  // 5 meters
    .dY = 0,
    .RightSpeed = 800,
    .LeftSpeed = 800
};

WB_NavEmulator_ConvertMelkensRoute(&melkens_step);
```

#### Obsługa komend WB Butler
```c
// Process WB drive requests
WB_NavEmulator_ProcessDriveRequest(WB_DRIVE_REQ_AUTO, 0.0f);

// Navigate to specific coordinates (WB style)
WB_NavEmulator_SetTarget(5.0f, 3.0f, 90.0f);

// Approach bay for feeding
WB_NavEmulator_ApproachBay(1);
```

### 2. Konfiguracja środowiska

#### Definicja torów (tracks)
```c
// Setup tracks in WB emulator
WB_TrackPos_t tracks[] = {
    {1, 0.0f, 0.0f, 0, 800, 800, 100},      // Track 1: origin
    {2, 5.0f, 0.0f, 90, 800, 800, 100},     // Track 2: 5m east, turn 90°
    {3, 5.0f, 5.0f, 180, 800, 800, 100},    // Track 3: corner
};
```

#### Definicja bays (miejsc karmienia)
```c
WB_Bay_t bays[] = {
    {1, 2.0f, 1.0f, 2.0f, 0.5f, 2.5f, 1.0f, 2.5f, 0.5f, 0.1f, 0.1f, 2.25f, 5000},
    {2, 4.0f, 1.0f, 4.0f, 0.5f, 4.5f, 1.0f, 4.5f, 0.5f, 0.1f, 0.1f, 4.25f, 5000},
};
```

#### Konfiguracja magnetów
```c
WB_ReferencePosition_t references[] = {
    {1, 1.0f, 0.0f, 0, 1, 50.0f},    // Reference 1: 1m, threshold 50
    {2, 3.0f, 0.0f, 0, 1, 50.0f},    // Reference 2: 3m, threshold 50
    {3, 5.0f, 0.0f, 90, 1, 50.0f},   // Reference 3: 5m, turn point
};
```

### 3. Symulator porównawczy

#### Podstawowe użycie
```python
# Import symulatora
from Robot_Navigation_Simulator import NavigationSimulator, SimulationVisualizer

# Utworzenie symulatora
simulator = NavigationSimulator(width=8.0, height=6.0)

# Utworzenie wizualizacji
visualizer = SimulationVisualizer(simulator)

# Start symulacji
simulator.start_comparison_simulation()
visualizer.start_animation()
```

#### Konfiguracja niestandardowych scenariuszy
```python
# Definiowanie trasy MELKENS
melkens_route = [
    RouteStep("NORM", 2.0, 0.0, 0.8, 0.0, 0.0),      # Forward 2m
    RouteStep("R_90", 0.0, 0.0, 0.5, 90.0, 0.0),     # Turn right 90°
    RouteStep("NORM", 3.0, 0.0, 0.8, 90.0, 0.0),     # Forward 3m
]

# Ustawienie celu WB
wb_target = Position(5.0, 3.0, 180.0)

# Uruchomienie porównania
simulator.melkens_navigator.set_route(melkens_route)
simulator.wb_navigator.set_target(wb_target)
```

---

## 📚 Dokumentacja API

### WB Navigation Emulator API

#### Core Functions

##### `void WB_NavEmulator_Init(void)`
Inicjalizuje emulator nawigacji WB.
- Zeruje kontekst nawigacji
- Ładuje domyślną konfigurację
- Resetuje statystyki

##### `void WB_NavEmulator_Update(void)`
Aktualizuje stan emulatora (wywołuj w pętli głównej).
- Przetwarza aktualny stan nawigacji
- Aktualizuje pozycję robota
- Sprawdza warunki timeout

##### `bool WB_NavEmulator_SetTarget(float x, float y, float heading)`
Ustawia cel nawigacji w współrzędnych świata.
- **Parametry**: `x, y` - współrzędne docelowe [m], `heading` - kierunek [°]
- **Zwraca**: `true` jeśli cel został ustawiony pomyślnie

##### `void WB_NavEmulator_SimulateMagnetDetection(MagnetName detectedMagnet)`
Symuluje detekcję magnesu z systemu MELKENS.
- **Parametry**: `detectedMagnet` - numer wykrytego magnesu (Magnet1-31)
- Konwertuje pozycję magnesu na format WB
- Stosuje korekcję magnetyczną

#### Navigation Functions

##### `bool WB_NavEmulator_NavigateToTrack(uint32_t trackId)`
Nawigacja do pozycji toru.
- **Parametry**: `trackId` - identyfikator toru docelowego
- **Zwraca**: `true` jeśli nawigacja została rozpoczęta

##### `bool WB_NavEmulator_ApproachBay(uint32_t bayId)`
Podejście do bay w celu karmienia.
- **Parametry**: `bayId` - identyfikator bay
- **Zwraca**: `true` jeśli podejście zostało rozpoczęte

##### `bool WB_NavEmulator_ExecuteFeeding(uint32_t bayId, float amount)`
Wykonanie operacji karmienia.
- **Parametry**: `bayId` - identyfikator bay, `amount` - ilość paszy [kg]
- **Zwraca**: `true` jeśli karmienie zostało rozpoczęte

#### Integration Functions

##### `bool WB_NavEmulator_ConvertMelkensRoute(const RouteStep* melkensStep)`
Konwertuje krok trasy MELKENS na nawigację WB.
- **Parametry**: `melkensStep` - wskaźnik na krok trasy MELKENS
- **Zwraca**: `true` jeśli konwersja się powiodła

##### `void WB_NavEmulator_ApplyToMelkensMotors(float speed, float steering)`
Aplikuje komendy WB do silników MELKENS.
- **Parametry**: `speed` - prędkość [0-1000], `steering` - skręt [-30 do +30°]

#### Diagnostic Functions

##### `void WB_NavEmulator_PrintStatus(void)`
Wyświetla aktualny status nawigacji.

##### `void WB_NavEmulator_GetStatistics(uint32_t* totalDistance, uint32_t* navigationTime, uint16_t* magnetDetections, uint8_t* errorCount)`
Pobiera statystyki nawigacji.

##### `const WB_NavigationContext_t* WB_NavEmulator_GetContext(void)`
Zwraca kontekst nawigacji do debugowania.

---

## 🧪 Testowanie

### Unit Tests

#### Test detekcji magnetów
```c
void test_magnet_detection(void) {
    WB_NavEmulator_Init();
    
    // Symuluj detekcję magnesu centralnego
    WB_NavEmulator_SimulateMagnetDetection(Magnet16);
    
    const WB_NavigationContext_t* ctx = WB_NavEmulator_GetContext();
    assert(ctx->magneticPositionValid == true);
    assert(ctx->magneticFieldStrength > 0.0f);
}
```

#### Test konwersji tras
```c
void test_route_conversion(void) {
    RouteStep step = {
        .OperationType = NORM,
        .dX = 200,  // 2m
        .dY = 0,
        .RightSpeed = 800,
        .LeftSpeed = 800
    };
    
    bool result = WB_NavEmulator_ConvertMelkensRoute(&step);
    assert(result == true);
    
    WB_WorldPosition_t pos = WB_NavEmulator_GetPosition();
    // Verify target was set correctly
}
```

### Integration Tests

#### Test pełnej sekwencji nawigacji
```c
void test_full_navigation_sequence(void) {
    WB_NavEmulator_Init();
    
    // 1. Ustaw cel
    bool result = WB_NavEmulator_SetTarget(5.0f, 3.0f, 90.0f);
    assert(result == true);
    
    // 2. Symuluj nawigację
    for (int i = 0; i < 100; i++) {
        WB_NavEmulator_Update();
        // Simulate magnet detections
        if (i % 10 == 0) {
            WB_NavEmulator_SimulateMagnetDetection(Magnet16);
        }
    }
    
    // 3. Sprawdź osiągnięcie celu
    WB_NavigationState_t state = WB_NavEmulator_GetState();
    assert(state == WB_NAV_STATE_IDLE);
}
```

### Symulator - Unit Tests

#### Test modelu robota
```python
def test_robot_movement():
    robot = Robot(Position(0, 0, 0))
    robot.velocity = 1.0  # 1 m/s
    robot.steering_angle = 0.0
    
    robot.update_position(1.0)  # 1 second
    
    assert abs(robot.position.x - 1.0) < 0.01
    assert abs(robot.position.y - 0.0) < 0.01
```

#### Test nawigacji MELKENS
```python
def test_melkens_navigation():
    robot = Robot(Position(0, 0, 0))
    navigator = MelkensNavigator(robot)
    
    route = [RouteStep("NORM", 2.0, 0.0, 0.8, 0.0, 0.0)]
    navigator.set_route(route)
    
    for _ in range(50):  # 5 seconds
        navigator.update(0.1)
        robot.update_position(0.1)
    
    assert abs(robot.position.x - 2.0) < 0.1
```

### Performance Tests

#### Test wydajności emulatora
```c
void test_emulator_performance(void) {
    uint32_t start_time = System_GetTimeMs();
    
    for (int i = 0; i < 1000; i++) {
        WB_NavEmulator_Update();
    }
    
    uint32_t elapsed = System_GetTimeMs() - start_time;
    assert(elapsed < 100);  // Should complete in <100ms
}
```

---

## 🔧 Rozwiązywanie problemów

### Częste problemy i rozwiązania

#### 1. Emulator nie odpowiada na detekcję magnetów

**Objawy:**
- Brak korekcji po detekcji magnesu
- `magneticPositionValid` zawsze `false`

**Rozwiązanie:**
```c
// Sprawdź czy emulacja jest włączona
WB_NavEmulator_SetEmulationMode(true);

// Sprawdź czy magnesy są prawidłowo wykrywane
WB_NavEmulator_PrintMagneticInfo();

// Sprawdź threshold detekcji
if (field_strength < 10.0f) {
    // Zbyt słabe pole magnetyczne
    // Dostosuj próg w WB_NavEmulator_ProcessMagneticField()
}
```

#### 2. Robot nie porusza się do celu

**Objawy:**
- `WB_NAV_STATE_NAVIGATING` ale brak ruchu
- `velocity` = 0 lub `steering_angle` = 0

**Rozwiązanie:**
```c
// Sprawdź czy cel jest ustawiony
const WB_NavigationContext_t* ctx = WB_NavEmulator_GetContext();
if (ctx->targetPos.x == 0 && ctx->targetPos.y == 0) {
    // Cel nie jest ustawiony
    WB_NavEmulator_SetTarget(5.0f, 3.0f, 90.0f);
}

// Sprawdź czy MotorManager jest aktywny
if (!MotorManager_IsInitialized()) {
    MotorManager_Init();
}

// Sprawdź błędy nawigacji
WB_NavEmulator_PrintStatus();
```

#### 3. Konwersja tras MELKENS nie działa

**Objawy:**
- `WB_NavEmulator_ConvertMelkensRoute()` zwraca `false`
- Brak ruchu po konwersji

**Rozwiązanie:**
```c
// Sprawdź prawidłowość struktury RouteStep
RouteStep step = {
    .OperationType = NORM,  // Sprawdź czy typ jest obsługiwany
    .dX = 200,             // Sprawdź czy wartości są > 0
    .dY = 0,
    .RightSpeed = 800,     // Sprawdź czy prędkości są w zakresie
    .LeftSpeed = 800,
    .Angle = 0.0f,
    .MagnetCorrection = 0.0f
};

// Debuguj konwersję
printf("Converting step: type=%d, dx=%.2f, dy=%.2f\n", 
       step.OperationType, step.dX/100.0f, step.dY/100.0f);
```

#### 4. Symulator Python nie uruchamia się

**Objawy:**
- Import errors
- Matplotlib errors

**Rozwiązanie:**
```bash
# Aktualizuj pakiety
pip install --upgrade numpy matplotlib

# Dla problémów z GUI
export MPLBACKEND=TkAgg
python Robot_Navigation_Simulator.py

# Alternatywnie użyj conda
conda install -c conda-forge matplotlib
```

#### 5. Błędy kompilacji w środowisku MELKENS

**Objawy:**
- Undefined reference errors
- Include path errors

**Rozwiązanie:**
```c
// Sprawdź czy wszystkie pliki są dołączone do projektu
// W MPLAB X: Project Properties -> Conf -> Categories: XC16 -> Directories

// Dodaj ścieżki include:
// Melkens_PMB/CANHandler/
// Melkens_PMB/

// Sprawdź czy brakuje implementacji
#ifndef WB_NAVIGATION_EMULATOR_H
#error "WB_Navigation_Emulator.h not found - check include paths"
#endif
```

### Debug Tools

#### 1. Status monitoring
```c
void debug_navigation_status(void) {
    WB_NavEmulator_PrintStatus();
    
    uint32_t distance, time;
    uint16_t detections;
    uint8_t errors;
    
    WB_NavEmulator_GetStatistics(&distance, &time, &detections, &errors);
    
    printf("Stats: dist=%lu, time=%lu, detections=%d, errors=%d\n",
           distance, time, detections, errors);
}
```

#### 2. Path visualization
```c
void debug_print_path(void) {
    const WB_NavigationContext_t* ctx = WB_NavEmulator_GetContext();
    
    printf("Current: (%.2f, %.2f) @ %.1f°\n",
           ctx->currentPos.x, ctx->currentPos.y, ctx->currentPos.heading);
    printf("Target:  (%.2f, %.2f) @ %.1f°\n", 
           ctx->targetPos.x, ctx->targetPos.y, ctx->targetPos.heading);
    printf("Distance remaining: %.2f m\n", ctx->remainingDistance);
    printf("Cross-track error: %.2f m\n", ctx->crossTrackError);
}
```

#### 3. Magnetic field debugging
```c
void debug_magnetic_field(void) {
    WB_NavEmulator_PrintMagneticInfo();
    
    // Check all reference positions
    for (int i = 0; i < reference_count; i++) {
        printf("Reference %d: (%.2f, %.2f) threshold=%.1f\n",
               reference_positions[i].id,
               reference_positions[i].posX,
               reference_positions[i].posY,
               reference_positions[i].fieldStrengthThreshold);
    }
}
```

### Logging System

```c
#define WB_LOG_DEBUG(fmt, ...) printf("[WB_DEBUG] " fmt "\n", ##__VA_ARGS__)
#define WB_LOG_INFO(fmt, ...)  printf("[WB_INFO] " fmt "\n", ##__VA_ARGS__)
#define WB_LOG_ERROR(fmt, ...) printf("[WB_ERROR] " fmt "\n", ##__VA_ARGS__)

// Usage examples:
WB_LOG_DEBUG("Magnet detected at position %.1f", position);
WB_LOG_INFO("Navigation target reached");
WB_LOG_ERROR("Path calculation failed");
```

---

## 📞 Wsparcie techniczne

### Kontakt
- **Email**: support@melkens.com
- **Documentation**: [Link do dokumentacji online]
- **Issues**: [Link do GitHub Issues]

### Zgłaszanie problemów
Przy zgłaszaniu problemów proszę dołączyć:

1. **Wersję oprogramowania**
2. **Logi debugowania**
3. **Kroki do reprodukcji**
4. **Oczekiwane vs rzeczywiste zachowanie**
5. **Konfigurację środowiska**

### Aktualizacje
- Regularnie sprawdzaj aktualizacje w repozytorium
- Śledź [CHANGELOG.md] dla informacji o nowych funkcjach
- Subskrybuj notyfikacje o nowych release'ach

---

**Copyright © 2024 MELKENS Integration Team. All rights reserved.**