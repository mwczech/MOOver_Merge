# 🤖 Plan Lokalnego Symulatora MOOver MELKENS z IMU

## 📋 Analiza Obecnej Sytuacji

### Obecna Architektura:
- **Web Simulator**: Działa na Railway (web-simulator/)
- **Hardware Melkens**: STM32G473 z IMU (Melkens/Melkens_IMU/)
- **Backend**: Express.js z WebSocket (backend/server.js)
- **Frontend**: React z real-time kontrolą

### Problem:
Railway nie obsługuje bezpośrednio urządzeń USB/serial, więc testowanie z prawdziwym IMU wymaga lokalnego rozwiązania.

## 🎯 Proponowana Architektura Hybrydowa

### 1. **Lokalna Stacja Testowa**
```
🖥️  Twój Komputer
├── 📦 Local Simulator (Python/Node.js)
├── 🔌 IMU via USB/Serial
├── 🌐 Web Interface (localhost)
└── 📡 Sync z Railway
```

### 2. **Architektura Multi-Environment**
```
🌍 Development Workflow:
├── 🌐 Railway (Web Development)    ← Cursor Web
├── 💻 Local (Hardware Testing)     ← Git Clone
└── 🔄 Sync Strategy               ← Automated
```

## 🛠️ Implementacja Lokalnego Symulatora

### **Opcja A: Python-based Local Simulator** (Rekomendowana)

#### Struktura:
```
local-simulator/
├── simulator.py           # Główny symulator
├── imu_interface.py       # Interface do IMU
├── route_engine.py        # Engine tras
├── web_server.py          # Local web interface
├── config.json           # Konfiguracja
├── requirements.txt      # Dependencies
└── routes/               # Definicje tras
    ├── route_A.json
    ├── route_B.json
    └── ...
```

#### Kluczowe komponenty:
- **Serial Communication**: pySerial dla komunikacji z IMU
- **Real-time Data**: IMU data streaming (gyro, accel, magnetometer)
- **Route Simulation**: Physics-based movement simulation
- **Web Interface**: Flask/FastAPI dla kontroli
- **Data Logging**: CSV/JSON logging tras

### **Opcja B: Node.js Extension** (Kompatybilna z obecnym kodem)

#### Rozszerzenie obecnego backendu:
```javascript
// local-adapter.js
const SerialPort = require('serialport');
const { RobotSimulator } = require('./backend/server.js');

class LocalIMUSimulator extends RobotSimulator {
    constructor() {
        super();
        this.imuPort = null;
        this.setupIMU();
    }
    
    async setupIMU() {
        // Auto-detect IMU device
        this.imuPort = new SerialPort('/dev/ttyUSB0', {
            baudRate: 115200
        });
    }
    
    processIMUData(data) {
        // Parse IMU data and update simulation
        // Update magnetometer, gyro, accelerometer
    }
}
```

## 🔄 Workflow Strategii

### **Strategia 1: Git-based Sync (Podstawowa)**
```bash
# Na twoim komputerze
git clone <repository>
cd <project>
npm run local-simulator

# Po testach
git add routes/
git commit -m "Update routes after local testing"
git push origin main
```

### **Strategia 2: Automated Sync (Zaawansowana)**
```bash
# Skrypt synchronizacji
#!/bin/bash
# sync-routes.sh

# Pobierz najnowsze zmiany
git pull origin main

# Uruchom lokalne testy
npm run test-routes

# Automatycznie prześlij rezultaty
git add .
git commit -m "Auto-sync: $(date)"
git push origin main
```

### **Strategia 3: Railway API Integration (Profesjonalna)**
```python
# railway_sync.py
import requests
import json

class RailwaySync:
    def __init__(self, api_key):
        self.api_key = api_key
        
    def upload_routes(self, routes_data):
        # Direct API call do Railway
        response = requests.post(
            'https://your-railway-app.com/api/routes',
            headers={'Authorization': f'Bearer {self.api_key}'},
            json=routes_data
        )
        return response.status_code == 200
```

## 🚀 Plan Wdrożenia

### **Faza 1: Setup Lokalnego Środowiska (Tydzień 1)**
1. **Sklonuj repository lokalnie**
2. **Zainstaluj dependencies**
3. **Skonfiguruj IMU connection**
4. **Test podstawowej komunikacji**

### **Faza 2: Local Simulator Development (Tydzień 2-3)**
1. **Implement IMU interface**
2. **Create route testing engine**
3. **Build local web interface**
4. **Add data logging**

### **Faza 3: Integration & Testing (Tydzień 4)**
1. **Test wszystkich tras z prawdziwym IMU**
2. **Optimize physics simulation**
3. **Setup automated sync**
4. **Documentation**

## 📱 Interfejs Użytkownika

### **Local Control Panel**
```
🖥️ Local Simulator Dashboard
├── 📊 IMU Status (Real-time)
├── 🗺️ Route Editor
├── ▶️ Test Controls
├── 📈 Performance Metrics
└── 🔄 Railway Sync Status
```

### **Typowy Workflow**
1. **Cursor Web**: Edytuj kod algorytmów
2. **Git**: Pobierz zmiany lokalnie  
3. **Local**: Test z prawdziwym IMU
4. **Validate**: Sprawdź performance
5. **Git**: Upload wyników do Railway

## 🔧 Konfiguracja Techniczna

### **IMU Requirements**
- **Connection**: USB/Serial (STM32 UART)
- **Protocol**: Custom binary/JSON protocol
- **Frequency**: 100Hz+ dla real-time
- **Data**: Gyro (3-axis), Accel (3-axis), Mag (3-axis)

### **Computer Requirements**
- **OS**: Linux/Windows/macOS
- **Port**: USB 2.0+ dla IMU
- **RAM**: 4GB+ (dla simulation)
- **Storage**: 1GB+ (logs, routes)

### **Network Setup**
```yaml
# docker-compose.yml dla local development
version: '3.8'
services:
  local-simulator:
    build: ./local-simulator
    ports:
      - "3001:3001"
    devices:
      - "/dev/ttyUSB0:/dev/ttyUSB0"  # IMU device
    volumes:
      - "./routes:/app/routes"
      - "./logs:/app/logs"
```

## 📊 Monitoring & Analytics

### **Local Metrics**
- IMU data quality
- Route completion rates  
- Performance benchmarks
- Hardware health

### **Sync Analytics**
- Upload/download speeds
- Conflict resolution
- Version tracking
- Railway deployment status

## 🆘 Troubleshooting

### **Częste Problemy**
1. **IMU nie jest wykrywany**
   - Sprawdź drivers
   - Verify permissions (`sudo usermod -a -G dialout $USER`)
   
2. **Sync conflicts**
   - Use feature branches
   - Implement merge strategies
   
3. **Performance issues**
   - Adjust IMU frequency
   - Optimize simulation loop

## 🎯 Rekomendacje

### **Best Practices**
1. **Always test locally** przed deployment
2. **Use version control** dla tras
3. **Document test results** 
4. **Maintain hardware calibration**
5. **Regular railway syncs**

### **Success Metrics**
- ✅ IMU data quality > 95%
- ✅ Route completion rate > 90%  
- ✅ Sync success rate > 99%
- ✅ Development velocity increase

---

**🎯 Następne Kroki:**
1. Potwierdź architekturę (Python vs Node.js)
2. Określ specyfikację IMU protocol
3. Setup development environment
4. Implement pierwszy prototyp

**📞 Do Decyzji:**
- Preferred programming language?
- IMU communication protocol details?
- Railway API access level?
- Timeline expectations?