# 🤖 MOOver MELKENS Local Simulator with Real IMU

Lokalna wersja symulatora MOOver MELKENS z obsługą prawdziwego IMU i synchronizacją z Railway.

## 🚀 Quick Start

### 1. **Klonowanie Repository**
```bash
# Sklonuj repository lokalnie
git clone <your-repository-url>
cd <project-directory>

# Przejdź do katalogu lokalnego symulatora  
cd local-simulator
```

### 2. **Instalacja Zależności**
```bash
# Zainstaluj Python dependencies
pip install -r requirements.txt

# Alternatywnie z virtualenv (rekomendowane)
python -m venv venv
source venv/bin/activate  # Linux/Mac
# venv\Scripts\activate     # Windows
pip install -r requirements.txt
```

### 3. **Konfiguracja**
```bash
# Uruchom interaktywną konfigurację
python start_local_simulator.py setup

# Lub z menu
python start_local_simulator.py
```

### 4. **Uruchomienie**
```bash
# Uruchom symulator
python start_local_simulator.py run

# Otwórz browser: http://localhost:5000
```

## 📋 Pełna Instrukcja

### **Przygotowanie Środowiska**

#### Requirements:
- **Python 3.8+** 
- **Git**
- **USB port** dla IMU
- **4GB+ RAM**
- **1GB storage**

#### Linux Setup:
```bash
# Dodaj użytkownika do grupy dialout (dla dostępu do USB)
sudo usermod -a -G dialout $USER
# Wyloguj się i zaloguj ponownie

# Sprawdź dostępne porty
ls /dev/ttyUSB* /dev/ttyACM*
```

#### Windows Setup:
```powershell
# Sprawdź Device Manager dla COM ports
# Zainstaluj drivers dla twojego IMU (STM32/ESP32)
```

### **Podłączenie IMU**

#### STM32G473 (Melkens IMU):
1. **Podłącz** IMU przez USB
2. **Sprawdź** czy device jest wykrywany:
   ```bash
   python start_local_simulator.py devices
   ```
3. **Skonfiguruj** baudrate (domyślnie 115200)

#### Protocol komunikacji:
```
Format wiadomości: "IMU,gx,gy,gz,ax,ay,az,mx,my,mz\n"
Gdzie:
- gx,gy,gz: Gyroscope (°/s)
- ax,ay,az: Accelerometer (g) 
- mx,my,mz: Magnetometer (μT)
```

### **Konfiguracja Tras**

#### Struktura pliku trasy:
```json
{
  "name": "Route A - Square Pattern",
  "waypoints": [
    {"x": 0, "y": 0, "speed": 50},
    {"x": 100, "y": 0, "speed": 50},
    {"x": 100, "y": 100, "speed": 50},
    {"x": 0, "y": 100, "speed": 50},
    {"x": 0, "y": 0, "speed": 30}
  ],
  "description": "Basic square navigation pattern"
}
```

#### Tworzenie nowych tras:
```bash
# Edytuj w katalogu routes/
nano routes/route_custom.json

# Lub używaj web interface: http://localhost:5000
```

### **Synchronizacja z Railway**

#### Konfiguracja:
```bash
# Ustaw Railway URL podczas setup
python start_local_simulator.py setup

# Ręczna konfiguracja w sync_config.json
{
  "railway_url": "https://your-app.up.railway.app",
  "auto_commit": true,
  "git_branch": "main"
}
```

#### Operacje sync:
```bash
# Sprawdź status
python start_local_simulator.py sync status

# Wyślij trasy do Railway
python start_local_simulator.py sync to-railway

# Pobierz trasy z Railway  
python start_local_simulator.py sync from-railway

# Sync dwukierunkowy
python start_local_simulator.py sync bidirectional
```

### **Workflow Developmentu**

#### Typowy cykl pracy:
```bash
# 1. Pobierz najnowsze zmiany
git pull origin main

# 2. Synchronizuj z Railway
python start_local_simulator.py sync from-railway

# 3. Uruchom lokalny symulator  
python start_local_simulator.py run

# 4. Testuj trasy z prawdziwym IMU
# [Manual testing przez web interface]

# 5. Wyślij zmiany do Railway
python start_local_simulator.py sync to-railway

# 6. Commit i push do Git
git add .
git commit -m "Updated routes after local testing"
git push origin main
```

#### Automated workflow:
```bash
# Skrypt automatyzujący cały proces
#!/bin/bash
git pull origin main
python start_local_simulator.py sync bidirectional
echo "✅ Environment synced and ready for testing"
```

## 🎯 Funkcje

### **Web Interface Features:**
- 📊 **Real-time IMU monitoring** (gyro, accel, magnetometer)
- 🕹️ **Virtual joystick** dla manual control
- 🗺️ **Route management** (load, start, stop)
- 📈 **Live position tracking** 
- 🧲 **16-sensor magnetometer visualization**
- 📝 **System logs** z timestampami
- ⚡ **Emergency stop** button

### **IMU Integration:**
- 🔌 **Auto-detection** USB/Serial devices
- 📡 **Real-time data streaming** (100Hz+)
- 🧭 **Sensor fusion** dla navigation
- 📊 **Data logging** w CSV/JSON
- 🔧 **Configurable** communication protocol

### **Railway Sync:**
- 🔄 **Bidirectional synchronization**
- 📦 **Automatic backups** przed sync
- 🚀 **Git integration** dla version control  
- 📈 **Status monitoring** local vs Railway
- ⚙️ **Conflict resolution**

### **Development Tools:**
- 🛠️ **CLI commands** dla wszystkich operacji
- 📱 **Device detection** utility
- 🔧 **Interactive setup** wizard
- 📊 **Performance monitoring**
- 🐛 **Debug logging** z różnymi poziomami

## 🆘 Troubleshooting

### **IMU nie jest wykrywany**
```bash
# Linux
sudo dmesg | grep tty
lsusb
ls -la /dev/ttyUSB*

# Sprawdź permissions
sudo chmod 666 /dev/ttyUSB0
```

### **Błędy komunikacji serial**
```bash
# Sprawdź czy port nie jest zajęty
sudo fuser /dev/ttyUSB0

# Kill proceso używające port
sudo fuser -k /dev/ttyUSB0

# Restart IMU
# Physically unplug and reconnect
```

### **Sync failures z Railway**
```bash
# Sprawdź connectivity
curl https://your-railway-app.up.railway.app/api/status

# Check Git status  
git status
git remote -v

# Reset local changes
git reset --hard HEAD
```

### **Performance issues**
```bash
# Reduce IMU frequency
# Edit baudrate in config: 57600 instead of 115200

# Check system resources
top
ps aux | grep python

# Clear logs
rm -rf logs/*
```

## 📊 Monitoring & Analytics

### **System Metrics:**
```bash
# Sprawdź status wszystkich komponentów
python start_local_simulator.py sync status

# Monitor w real-time
tail -f logs/simulator.log
```

### **IMU Quality Metrics:**
- **Data rate**: >90Hz for optimal performance
- **Latency**: <10ms serial communication  
- **Accuracy**: ±0.1° magnetometer heading
- **Stability**: <1% dropped packets

### **Performance Targets:**
- ✅ **Route completion**: >95% success rate
- ✅ **Sync reliability**: >99% success rate  
- ✅ **Response time**: <50ms UI updates
- ✅ **Memory usage**: <500MB total

## 🔧 Advanced Configuration

### **Custom IMU Protocol:**
```python
# Edit simulator.py - parse_imu_data method
def parse_imu_data(self, line: str) -> Optional[IMUData]:
    # Custom protocol implementation
    if line.startswith("CUSTOM,"):
        # Your parsing logic here
        pass
```

### **Route Algorithm Customization:**
```python
# Edit simulator.py - update_route_progress method  
def update_route_progress(self, robot_state, imu_data):
    # Custom navigation algorithm
    # Use real IMU feedback for corrections
    pass
```

### **Railway API Extensions:**
```python
# Add custom endpoints in web-simulator/backend/server.js
app.post('/api/custom_command', (req, res) => {
    // Handle custom commands from local simulator
});
```

## 📚 API Reference

### **Local Simulator API:**
```
GET  /api/status          # System status
GET  /api/routes          # Available routes  
POST /api/start_route     # Start autonomous route
POST /api/emergency_stop  # Emergency stop
```

### **WebSocket Events:**
```javascript
// Client -> Server
socket.emit('control_robot', {leftSpeed, rightSpeed});
socket.emit('emergency_stop');

// Server -> Client  
socket.on('robot_status', data => { /* robot state */ });
socket.on('imu_data', data => { /* IMU measurements */ });
```

### **Railway Integration:**
```bash
# CLI commands
python sync_with_railway.py status
python sync_with_railway.py to-railway
python sync_with_railway.py from-railway  
python sync_with_railway.py bidirectional
```

---

## 📞 Support

### **Common Issues:**
1. **Permission denied** - Add user to dialout group
2. **Port busy** - Check for other processes  
3. **Sync conflicts** - Use bidirectional sync
4. **IMU not responding** - Check baudrate/connections

### **Getting Help:**
- 📖 Check logs: `logs/simulator.log`
- 🐛 Enable debug: `LOG_LEVEL=DEBUG python start_local_simulator.py run`
- 📱 Test IMU: `python start_local_simulator.py devices`

---

**🎯 Happy Testing!** 🤖

*Lokalna stacja testowa gotowa do pracy z prawdziwym IMU i synchronizacją z Railway deployment.*