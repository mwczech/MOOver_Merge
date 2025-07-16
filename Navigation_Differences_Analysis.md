# Analiza różnic w cyklu jazdy i nawigacji: MELKENS vs WB

## Podsumowanie wykonawcze

| Aspekt | MELKENS | WB (Wasserbauer) | Różnica |
|--------|---------|------------------|---------|
| **Architektura** | Step-based routes | World coordinate system | Fundamentalna |
| **Nawigacja** | Pure pursuit + magnet correction | Track/Bay coordinate navigation | Różne algorytmy |
| **Pozycjonowanie** | Relative positioning | Absolute world coordinates | Różne systemy odniesienia |
| **Korekcja magnetyczna** | Predefined corrections | Dynamic field strength | Różne metody |
| **Cykl jazdy** | State machine (6 states) | Butler state machine | Podobne, różne implementacje |

---

## 📊 Szczegółowa analiza różnic

### 1. **Architektura nawigacji**

#### MELKENS - Step-based Navigation
```c
typedef struct RouteStep_t {
    uint8_t OperationType;     // NORM, TU_L, TU_R, L_90, R_90, DIFF
    uint16_t dX, dY;          // Relative distance changes
    uint16_t RightSpeed, LeftSpeed;
    uint8_t DirectionRight, DirectionLeft;
    float Angle;
    float MagnetCorrection;   // Predefined correction values
} RouteStep;

// Navigation states
typedef enum RouteStates_t {
    RouteState_Init = 0,
    RouteState_Idle,
    RouteState_WaitForStart,
    RouteState_BuzzerLampIndication,
    RouteState_SetNextStep,
    RouteState_Drive
} RouteStates;
```

#### WB - World Coordinate Navigation  
```c
// From DDMap.cfg analysis
typedef struct {
    uint32_t trackId;
    float posX, posY;         // Absolute world coordinates
    uint16_t direction;       // World direction
    uint16_t trommelSpeed;
    uint16_t butlerSpeed;
} TrackPos;

typedef struct {
    uint32_t bayId;
    float entryNearX, entryNearY;    // Bay entry coordinates
    float exitFarX, exitFarY;        // Bay exit coordinates
    float offsetFar, offsetNear;     // Position offsets
    float feedPos;                   // Feeding position
} Bay;

// Butler states from CANopen
typedef struct {
    uint16_t driveRequest;     // WB_DRIVE_REQ_*
    uint16_t butlerState;      // Current operation state
    float driveLength;         // Distance to drive
    uint8_t abortRequest;      // Emergency stop
} WB_Butler_Control_t;
```

### 2. **Algorytmy nawigacji**

#### MELKENS - Pure Pursuit Algorithm
```c
// From Navigation.c
#define ROUTE_POINTS_DISTANCE 10  // 10cm between route points
uint8_t pursuitPointIncrement = 5; // Look-ahead distance
uint16_t closestPoint = 0;         // Closest route point
uint16_t pursuitPoint = 0;         // Target pursuit point

// Navigation flow:
// 1. Generate route points from steps (10cm intervals)
// 2. Find closest point to robot
// 3. Select pursuit point ahead 
// 4. Calculate angle to pursuit point
// 5. Control motors with differential steering
```

#### WB - Track/Bay Navigation
```c
// WB Navigation approach (inferred from config):
// 1. Load track positions from database
// 2. Navigate between absolute world coordinates
// 3. Use bay entry/exit points for precise positioning
// 4. Apply magnetic field strength corrections
// 5. Coordinate-based path planning

typedef struct {
    float coordX, coordY;          // Current world position
    float setCoordX, setCoordY;    // Target world position
    float yawDeg;                  // Current orientation
    float setYawDeg;               // Target orientation
} WB_WorldNavigation;
```

### 3. **System detekcji magnetów**

#### MELKENS - Discrete Magnet Array
```c
typedef enum MagnetName_t {
    Magnet1 = 0,    // Position -15
    // ... 
    Magnet16,       // Zero position (center)
    // ...
    Magnet31        // Position +15
} MagnetName;

// Predefined corrections
#define dMAGNET_R5  5*2.17     // Right 5 positions
#define dMAGNET_L5  -5*2.17    // Left 5 positions
#define dMAGNET_MID 0          // Center position

// Simple correction logic:
if (detected_magnet != Magnet16) {
    correction = predefined_correction[detected_magnet];
    apply_correction(correction);
}
```

#### WB - Field Strength Based
```c
// From WB config
typedef struct {
    uint32_t id;
    float posX, posY;
    uint16_t direction;
    uint16_t state;
    float fieldStrengthThreshold;  // Dynamic threshold
} ReferencePosition;

// WB magnetic detection (inferred):
// 1. Continuous field strength measurement
// 2. Dynamic threshold adjustment
// 3. Position interpolation between reference points
// 4. Smooth correction application
```

### 4. **Cykl jazdy i stany**

#### MELKENS - Route State Machine
```c
enum {
    RouteState_Init,              // Initialize route
    RouteState_Idle,              // Waiting for activation
    RouteState_WaitForStart,      // Waiting for start command
    RouteState_BuzzerLampIndication, // Audio/visual indication
    RouteState_SetNextStep,       // Load next route step
    RouteState_Drive              // Execute movement
};

// Execution flow:
// Init → Idle → WaitForStart → BuzzerLamp → SetNextStep → Drive → SetNextStep → ...
```

#### WB - Butler State Machine
```c
typedef enum {
    WB_BUTLER_STATE_STOPPED     = 0x00,
    WB_BUTLER_STATE_RUNNING     = 0x01,
    WB_BUTLER_STATE_MANUAL      = 0x02,
    WB_BUTLER_STATE_PAUSED      = 0x03,
    WB_BUTLER_STATE_ERROR       = 0x04,
    WB_BUTLER_STATE_TEACHING    = 0x05,
    WB_BUTLER_STATE_PARKING     = 0x06
} WB_ButlerState_t;

// WB execution flow:
// Stopped → Running → (Manual/Teaching/Parking) → Paused → Running → ...
```

---

## 🔄 Różnice w wykonaniu operacji

### Operacja: "Jedź prosto 5 metrów"

#### MELKENS Implementation:
```c
RouteStep step = {
    .OperationType = NORM,
    .dX = 500,  // 5 meters in cm
    .dY = 0,
    .RightSpeed = 800,
    .LeftSpeed = 800,
    .DirectionRight = R_FOR,
    .DirectionLeft = L_FOR,
    .ThumbleEnabled = TH_OFF,
    .Angle = 0.0,
    .MagnetCorrection = dMAGNET_NO_CORRECTION
};

// Execution:
// 1. SetNextStep loads parameters
// 2. Motors start with specified speeds
// 3. Distance tracked by encoder counts
// 4. Magnet detection provides correction
// 5. Stop when target distance reached
```

#### WB Implementation:
```c
// WB approach (inferred):
WB_Butler_Control_t cmd = {
    .driveRequest = WB_DRIVE_REQ_AUTO,
    .driveLength = 5.0,  // 5 meters
    .butlerState = WB_BUTLER_STATE_RUNNING
};

TrackPos target = {
    .posX = current_x + 5.0,  // Target world coordinate
    .posY = current_y,
    .direction = current_direction,
    .butlerSpeed = 800
};

// Execution:
// 1. Calculate world coordinates for target
// 2. Navigate using coordinate-based path planning
// 3. Continuous position updates from odometry + magnets
// 4. Smooth trajectory generation
// 5. Stop at target coordinates
```

### Operacja: "Skręć w lewo 90°"

#### MELKENS Implementation:
```c
RouteStep step = {
    .OperationType = L_90,
    .dX = 0,
    .dY = 0,
    .RightSpeed = 600,
    .LeftSpeed = 200,  // Differential steering
    .DirectionRight = R_FOR,
    .DirectionLeft = L_FOR,
    .Angle = -90.0,
    .MagnetCorrection = dMAGNET_MID
};

// Fixed differential speed for 90° turn
```

#### WB Implementation:
```c
WB_PDO_Manual_t control = {
    .speed = 400,      // Base speed
    .steering = -100,  // Full left steering
    .setYawDeg = current_yaw - 90.0
};

// Dynamic steering adjustment based on IMU feedback
```

---

## 🎯 Kluczowe różnice operacyjne

| Aspekt | MELKENS | WB | Implikacje |
|--------|---------|-------|------------|
| **Planowanie trasy** | Predefiniowane kroki | Dynamiczne współrzędne | WB bardziej elastyczny |
| **Korekcja błędów** | Dyskretne wartości | Ciągłe dostosowanie | WB bardziej precyzyjny |
| **Pozycjonowanie** | Względne przemieszczenia | Absolutne współrzędne | WB lepsze dla dużych obszarów |
| **Reakcja na przeszkody** | Restart sekwencji | Przeliczenie trasy | WB bardziej inteligentny |
| **Kalibracja** | Ręczne ustawienia | Automatyczne uczenie | WB łatwiejszy w konfiguracji |

---

## 📈 Zalety i wady każdego systemu

### MELKENS - Step-based
**✅ Zalety:**
- Prostota implementacji
- Przewidywalność wykonania
- Łatwe debugowanie
- Niskie wymagania obliczeniowe
- Deterministyczne zachowanie

**❌ Wady:**
- Ograniczona elastyczność
- Trudna adaptacja do zmian środowiska
- Kumulacja błędów pozycjonowania
- Brak automatycznej kompensacji
- Trudna konfiguracja nowych tras

### WB - Coordinate-based
**✅ Zalety:**
- Wysoka precyzja pozycjonowania
- Elastyczne planowanie tras
- Automatyczna kompensacja błędów
- Łatwa konfiguracja przez GUI
- Zaawansowane algoritmy nawigacji

**❌ Wady:**
- Większa złożoność implementacji
- Wyższe wymagania obliczeniowe
- Zależność od precyzyjnej kalibracji
- Potrzeba ciągłej aktualizacji pozycji
- Większe możliwości błędów systemowych

---

## 🔧 Rekomendacje dla integracji

1. **Hybrid Approach**: Połączenie prostoty MELKENS z precyzją WB
2. **Gradual Migration**: Stopniowe wprowadzanie funkcji WB
3. **Fallback Mechanism**: MELKENS jako backup dla WB
4. **Configuration Layer**: Wspólny interfejs konfiguracji
5. **Testing Framework**: Symulator dla obu systemów

---

## 📋 Plan migracji

### Faza 1: Kompatybilność
- [x] Implementacja warstwy CANopen WB
- [x] Mapowanie podstawowych funkcji
- [ ] Stub functions dla emulacji WB

### Faza 2: Funkcjonalność  
- [ ] World coordinate navigation
- [ ] Advanced magnet detection
- [ ] Dynamic path planning

### Faza 3: Optymalizacja
- [ ] Performance improvements
- [ ] Advanced algorithms
- [ ] Full WB compatibility

Następnym krokiem będzie implementacja stubów emulujących zachowanie WB oraz symulatora porównawczego.