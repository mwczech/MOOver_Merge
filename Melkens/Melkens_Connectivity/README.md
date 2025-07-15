# ESP32s3 Connectivity

This project implements a simple web server on an ESP32s3 board that allows:

- Connecting to a Wi-Fi network and provides a Web Server
- Uploading configuration data via web interface
- Performing OTA firmware updates directly from a browser
- Uploading additional firmware for external PMB board
- PMB control via UART commands
- Sending messages to MQTT broker

---

## 🚀 Configuration Overview

On boot, the ESP32 tries to connect to a saved Wi-Fi network. If it fails or no configuration is found, it starts in **Access Point (AP) mode**, allowing the user to configure the network via a browser.

### 🔧 Setup Flow

1. **First Boot / Reset**
   - ESP32 attempts to load configuration from file (e.g., `config.json` in LittleFS).
   - If no valid config is found or it will not be possible to connect to the defined network for 10 seconds, the device switches to **AP mode**.

2. **Access Point Mode**
   - SSID: `MELKENS-WIFI_` + MAC address (or other name configured in code)
   - No password (open network)
   - IP address: `192.168.4.1`
   - User navigates to `http://192.168.4.1` to access the **settings page**.

3. **Settings Page**
   - Accessible at `/settings`
   - Allows entering:
     - Wi-Fi SSID and password
     - MQTT broker IP
     - Static IP configuration
   - Submits config via `POST /submit`
   - Device saves settings and restarts.

4. **Normal Operation (Station Mode)**
   - ESP connects to the configured Wi-Fi network using the saved credentials and static IP (if provided).
   - Serves the main web interface.

---

## 🧩 Available HTTP Interfaces

| Method | Endpoint       | Description                          |
|--------|----------------|--------------------------------------|
| GET    | `/`            | Displays the main page               |
| GET    | `/settings`    | Displays the configuration page      |
| POST   | `/submit`      | Submits Wi-Fi & MQTT configuration   |
| POST   | `/updateEsp`   | Uploads firmware for ESP (OTA)       |
| POST   | `/config`      | Uploads a new config JSON file       |
| POST   | `/updatePmb`   | Uploads firmware for PMB module      |

---

## 🛠 OTA Firmware Build and Update

- To generate the firmware file in arduino IDE press: Sketch->Export Compiled Binary.
- the example path to the file is .../Melkens_Connectivity/build/esp32.esp32.esp32da/Melkens_Connectivity.ino.bin
- Firmware can be uploaded directly from the browser (no need for serial connection).
- ESP will reboot after a successful update.

---

## 📁 File System (LittleFS)

- Configuration file (e.g., `config.json`) is stored in LittleFS.
- Uploading a new config via `/config` replaces the file and restarts the ESP.
- The sample file is in: .../Melkens_Connectivity/data/config.json

---

