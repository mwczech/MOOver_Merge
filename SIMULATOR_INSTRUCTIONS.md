# Instrukcje uruchomienia symulatora nawigacji robotów

## Szybki start

### 1. Instalacja zależności
```bash
pip install numpy matplotlib
```

### 2. Uruchomienie symulatora
```bash
python Robot_Navigation_Simulator.py
```

## Co zobaczycie

### Okno symulatora
- **Lewa strona**: Wizualizacja 2D z robotami
  - **Niebieski robot**: MELKENS (step-based navigation)
  - **Czerwony robot**: WB (coordinate-based navigation)
  - **Niebieskie kropki**: Punkty toru (tracks)
  - **Zielone kwadraty**: Bay (miejsca karmienia)
  - **Czerwone kropki**: Markery magnetyczne

- **Prawa strona**: Wykres porównawczy metryk
  - Dystans przebyta przez oba roboty w czasie

### Wyniki symulacji
Po zakończeniu w konsoli zostaną wyświetlone:
```
============================================================
SIMULATION RESULTS
============================================================

MELKENS (Step-based) Results:
  distance_traveled: 12.450
  time_elapsed: 15.200
  energy_consumed: 18.750
  magnet_detections: 8
  navigation_errors: 0
  efficiency: 0.664

WB (Coordinate-based) Results:
  distance_traveled: 11.200
  time_elapsed: 13.800
  energy_consumed: 16.340
  magnet_detections: 12
  navigation_errors: 0
  efficiency: 0.685

Comparison:
  Distance efficiency: WB vs MELKENS = 0.900
  Time efficiency: WB vs MELKENS = 0.908
  Energy efficiency: WB vs MELKENS = 1.032
============================================================
```

## Interpretacja wyników

### Metryki wydajności
- **distance_traveled**: Całkowity dystans przejechany [m]
- **time_elapsed**: Czas nawigacji [s] 
- **energy_consumed**: Energia zużyta [jednostka względna]
- **magnet_detections**: Liczba detekcji magnetów
- **efficiency**: Stosunek dystansu do energii

### Analiza porównawcza
- **Distance efficiency < 1.0**: WB przejeżdża krótszą trasę
- **Time efficiency < 1.0**: WB jest szybszy
- **Energy efficiency > 1.0**: WB jest bardziej efektywny energetycznie

## Customizacja scenariuszy

### Zmiana parametrów robota
```python
# W pliku Robot_Navigation_Simulator.py
ROBOT_WIDTH = 0.8  # szerokość robota [m]
MAX_SPEED = 1.0    # maksymalna prędkość [m/s]
MAX_STEERING_ANGLE = 30  # maksymalny kąt skrętu [°]
```

### Modyfikacja trasy MELKENS
```python
def create_melkens_route(self) -> List[RouteStep]:
    return [
        RouteStep("NORM", 3.0, 0.0, 0.8, 0.0, 0.0),    # Jedź prosto 3m
        RouteStep("L_90", 0.0, 0.0, 0.5, -90.0, 0.0),  # Skręć w lewo 90°
        RouteStep("NORM", 2.0, 0.0, 0.8, -90.0, 0.0),  # Jedź prosto 2m
    ]
```

### Zmiana celu WB
```python
# W start_comparison_simulation()
self.wb_navigator.set_target(Position(8.0, 5.0, 180.0))  # x, y, heading
```

## Troubleshooting

### Problem: "No module named matplotlib"
```bash
pip install matplotlib
# lub
conda install matplotlib
```

### Problem: Okno symulatora nie otwiera się
```bash
export MPLBACKEND=TkAgg
python Robot_Navigation_Simulator.py
```

### Problem: Symulator działa zbyt wolno
```python
# Zmniejsz SIMULATION_TIME_STEP w pliku
SIMULATION_TIME_STEP = 0.05  # zamiast 0.1
```

## Zaawansowane funkcje

### Export danych do analizy
```python
# Dodaj na końcu main()
def export_results():
    melkens_metrics = simulator.melkens_robot.get_metrics()
    wb_metrics = simulator.wb_robot.get_metrics()
    
    import json
    results = {
        "melkens": melkens_metrics,
        "wb": wb_metrics,
        "timestamp": time.time()
    }
    
    with open("simulation_results.json", "w") as f:
        json.dump(results, f, indent=2)
    
    print("Results exported to simulation_results.json")
```

### Automatyczne testy wielu scenariuszy
```python
scenarios = [
    {"route_length": 5.0, "turns": 2},
    {"route_length": 10.0, "turns": 4}, 
    {"route_length": 15.0, "turns": 6}
]

for scenario in scenarios:
    simulator = NavigationSimulator()
    # Setup scenario...
    simulator.start_comparison_simulation()
    # Run without visualization for faster execution
```

## Przykłady użycia

### Scenario 1: Prosta trasa
```python
melkens_route = [
    RouteStep("NORM", 5.0, 0.0, 0.8, 0.0, 0.0),  # 5m prosto
]
wb_target = Position(5.0, 1.0, 0.0)
```

### Scenario 2: Kompleksowa trasa z karmienie
```python
melkens_route = [
    RouteStep("NORM", 2.0, 0.0, 0.8, 0.0, 0.0),      # Do pierwszego bay
    RouteStep("TU_R", 0.5, 0.0, 0.5, 45.0, 0.0),     # Podejście do bay
    RouteStep("NORM", 1.0, 0.0, 0.3, 45.0, 0.0),     # Wejście do bay
    RouteStep("NORM", -1.0, 0.0, 0.3, 45.0, 0.0),    # Wyjście z bay
    RouteStep("TU_L", 0.5, 0.0, 0.5, 0.0, 0.0),      # Powrót na tor
]
```

### Scenario 3: Test precyzji magnetycznej
```python
# Dodaj więcej markerów magnetycznych
for i in range(20):
    x = i * 0.5  # Co 50cm
    simulator.magnet_markers.append(MagnetMarker(i+100, x, 1.0, 75.0))
```

---

**Uwaga**: Symulator jest narzędziem analitycznym - rzeczywiste wyniki mogą się różnić w zależności od warunków fizycznych i implementacji sprzętowej.