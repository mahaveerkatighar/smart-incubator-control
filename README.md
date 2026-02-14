# 🔬 Smart Incubator Control System

A medical-grade IoT-based incubator monitoring and control system designed for NICU applications. Features real-time temperature monitoring, automated PID control, and emergency alert notifications.

![Version](https://img.shields.io/badge/version-5.1-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-ESP32-red)

## 📋 Overview

This project implements a complete incubator monitoring and control system for neonatal intensive care units (NICU). The system uses ESP32 microcontroller with cloud connectivity to provide real-time temperature monitoring, automated control, and emergency alerts to medical staff.

## ✨ Features

### Core Functionality
- ✅ **Real-time Temperature Monitoring** - DS18B20 digital sensor with ±0.5°C accuracy
- ✅ **PID Temperature Control** - Automated heating/cooling with deadband control
- ✅ **Dual Control Modes** - Automatic PID control and Manual override
- ✅ **Web Dashboard** - Beautiful, responsive real-time monitoring interface
- ✅ **Firebase Integration** - Cloud-based data storage and remote control
- ✅ **Email Alerts** - Emergency notifications to medical staff
- ✅ **LCD Display** - 16x2 I2C display with rotating information screens
- ✅ **Power Failure Detection** - Automatic alerts when device goes offline

### Safety Systems
- 🛡️ **Multi-level safety monitoring**
- 🛡️ **Critical temperature shutdown** (34.0°C - 38.5°C range)
- 🛡️ **Sensor fault detection**
- 🛡️ **Visual and audio alerts** (LEDs + Buzzer)
- 🛡️ **Emergency stop function**
- 🛡️ **Historical data logging**

## 🏗️ System Architecture
```
┌─────────────┐
│   ESP32     │
│ Controller  │
└──────┬──────┘
       │
       ├─── DS18B20 Temperature Sensor
       ├─── 16x2 LCD Display (I2C)
       ├─── 2-Channel Relay Module
       ├─── Status LEDs (Red/Green/Yellow)
       ├─── Buzzer
       └─── Mode Button
       
       │ WiFi
       ↓
       
┌─────────────┐      ┌──────────────┐
│  Firebase   │◄────►│ Web Dashboard│
│ Realtime DB │      │   (Browser)  │
└──────┬──────┘      └──────────────┘
       │
       ↓
       
┌─────────────┐
│   Python    │
│Alert System │
└─────────────┘
       │
       ↓ Email
       
┌─────────────┐
│  Medical    │
│    Staff    │
└─────────────┘
```

## 🔧 Hardware Requirements

| Component | Specification | Quantity |
|-----------|--------------|----------|
| ESP32 Development Board | 30-pin or 38-pin | 1 |
| DS18B20 Temperature Sensor | Waterproof, digital | 1 |
| 4.7kΩ Resistor | Pull-up for DS18B20 | 1 |
| 16x2 LCD Display | I2C module | 1 |
| 2-Channel Relay Module | 5V, Active LOW | 1 |
| LEDs | Red, Green, Yellow | 3 |
| 220Ω Resistors | For LEDs | 3 |
| Active Buzzer | 5V | 1 |
| Push Button | Momentary switch | 1 |
| Power Supply | 5V/2A USB | 1 |
| Breadboard/PCB | For prototyping | 1 |
| Jumper Wires | Male-to-Female | 20+ |

### Pin Connections
```
ESP32 GPIO  →  Component
─────────────────────────
GPIO 4      →  DS18B20 Data (with 4.7kΩ pull-up to 3.3V)
GPIO 21     →  LCD SDA
GPIO 22     →  LCD SCL
GPIO 32     →  Heater Relay IN1
GPIO 33     →  Cooler Relay IN2
GPIO 26     →  Red LED (via 220Ω resistor)
GPIO 27     →  Green LED (via 220Ω resistor)
GPIO 14     →  Yellow LED (via 220Ω resistor)
GPIO 13     →  Buzzer
GPIO 35     →  Mode Button (with pull-up)
5V          →  VCC (Relay, LCD, Sensors)
3.3V        →  DS18B20 VCC
GND         →  Common Ground (All components)
```

## 💻 Software Requirements

### Arduino IDE Setup

**Required Libraries:**
```
OneWire v2.3.7+
DallasTemperature v3.9.0+
Firebase ESP Client v4.3.0+ (by Mobizt)
LiquidCrystal I2C v1.1.2+
```

**Installation Steps:**
1. Open Arduino IDE
2. Go to **Sketch → Include Library → Manage Libraries**
3. Search and install each library listed above
4. Select **Board**: ESP32 Dev Module
5. Select appropriate **Port**

### Python Setup (For Email Alerts)

**Requirements:**
- Python 3.8 or later
- pip package manager

**Install Dependencies:**
```bash
pip install firebase-admin
```

## 📥 Installation & Setup

### 1. Clone Repository
```bash
git clone https://github.com/YOUR_USERNAME/smart-incubator-control.git
cd smart-incubator-control
```

### 2. Firebase Setup

1. Create Firebase project at https://console.firebase.google.com/
2. Enable **Realtime Database**
3. Get Firebase configuration from Project Settings
4. Download service account key for Python alerts

### 3. Configure Credentials

**For Arduino** - Create `config.h`:
```cpp
#define WIFI_SSID "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define API_KEY "your_firebase_api_key"
#define DATABASE_URL "your_firebase_database_url"
#define USER_EMAIL "your_email@example.com"
#define USER_PASSWORD "your_firebase_password"
```

**For Python** - Create `config.py`:
```python
SENDER_EMAIL = "your_email@gmail.com"
SENDER_PASSWORD = "your_gmail_app_password"
FIREBASE_CONFIG = {"databaseURL": "your_database_url"}
SERVICE_ACCOUNT_PATH = "serviceAccountKey.json"
```

**For Web Dashboard** - Edit `dashboard.html`:
Update Firebase config (around line 520)

### 4. Upload Code

1. Open `incubator_control.ino` in Arduino IDE
2. Create `config.h` with your credentials
3. Verify and Upload to ESP32
4. Open Serial Monitor (115200 baud) to verify operation

### 5. Run Dashboard

Open `dashboard.html` in any modern web browser

### 6. Start Alert System (Optional)
```bash
cd python-alerts
python incubator_alerts.py
```

## ⚙️ Configuration

### Temperature Thresholds
```cpp
targetTemp = 36.5;              // Default target temperature
CRITICAL_LOW_TEMP = 34.0;       // Emergency shutdown if below
CRITICAL_HIGH_TEMP = 38.5;      // Emergency shutdown if above
MIN_SAFE_TEMP = 35.0;           // Warning threshold low
MAX_SAFE_TEMP = 37.5;           // Warning threshold high
```

### Alert Recipients

Edit in `config.py`:
```python
ALERT_CONTACTS = {
    "doctors": [
        {"name": "Dr. Name", "email": "doctor@hospital.com", "phone": "+1-xxx"}
    ],
    "nurses": [
        {"name": "Nurse Name", "email": "nurse@hospital.com", "phone": "+1-xxx"}
    ]
}
```

## 🎮 Usage

### Operating Modes

**Auto Mode (Default)**
- System automatically maintains target temperature
- PID algorithm controls heater/cooler
- Continuous monitoring and adjustment

**Manual Mode**
- Press physical mode button OR use dashboard
- Manually control heater/cooler
- Useful for testing and troubleshooting

### Dashboard Features

- **Live Temperature Display** - Updates every 2 seconds
- **Historical Chart** - Last 60 readings visualization
- **Target Adjustment** - Set desired temperature via slider
- **Manual Control** - Toggle heater/cooler (manual mode only)
- **Mode Switching** - Switch between Auto/Manual
- **Emergency Stop** - Immediate system shutdown

### LCD Display

Cycles through 3 screens every 5 seconds:

1. **Temperature Screen** - Current and target temperature
2. **Control Screen** - Mode and actuator status
3. **Status Screen** - System status and alerts

### LED Indicators

| LED | Status | Meaning |
|-----|--------|---------|
| 🟢 Green | ON | Normal operation |
| 🟡 Yellow | ON | Warning/Standby |
| 🔴 Red | ON | Alert/Critical condition |

## 🛡️ Safety Features

### Temperature Monitoring
```
Temperature Range:
├─ Below 34.0°C    → EMERGENCY SHUTDOWN + Alert
├─ 34.0-35.0°C     → Warning alert, heating active
├─ 35.0-37.5°C     → Normal operation zone
├─ 37.5-38.5°C     → Warning alert, cooling active
└─ Above 38.5°C    → EMERGENCY SHUTDOWN + Alert
```

### Fault Detection

- **Sensor Disconnection** - Detects invalid readings (-127°C)
- **Power Failure** - No data >15 seconds triggers alert
- **Network Loss** - System continues local operation
- **Firebase Disconnection** - Local control maintained

### Emergency Procedures

If critical alert occurs:
1. ✅ Check infant immediately
2. ✅ Verify incubator display readings
3. ✅ Check with backup thermometer
4. ✅ Contact biomedical engineering
5. ✅ Document all actions taken

## 📊 Data Structure (Firebase)
```json
{
  "City_Hospital": {
    "devices": {
      "INCUBATOR_001": {
        "current": {
          "currentTemp": 36.5,
          "targetTemp": 36.5,
          "status": "NORMAL",
          "heaterState": false,
          "coolerState": false,
          "mode": "AUTO",
          "alertActive": false,
          "timestamp": "2024-02-14_10-30-00"
        },
        "settings": {
          "targetTemp": 36.5
        },
        "commands": {
          "setTargetTemp": 36.5,
          "setMode": "AUTO"
        },
        "history": {}
      }
    }
  }
}
```

## 🐛 Troubleshooting

### Common Issues

**Temperature reads -127°C**
- Check DS18B20 wiring
- Verify 4.7kΩ pull-up resistor is connected
- Test sensor separately

**Relays not switching**
- Verify 5V power supply (minimum 2A)
- Check relay board is Active LOW type
- Try different GPIO pins
- Test relays manually with jumper wire

**Firebase not connecting**
- Verify WiFi credentials
- Check Firebase API key and database URL
- Ensure Firebase Authentication is enabled
- Check internet connection

**LCD blank screen**
- Try I2C address 0x3F instead of 0x27
- Check SDA/SCL wiring
- Adjust contrast potentiometer on LCD

**Email alerts not working**
- Use Gmail App Password (not regular password)
- Enable 2-Factor Authentication first
- Check firewall settings
- Verify `serviceAccountKey.json` is present

## 👥 Team

This project was developed by:

- **Mahaveer Katighar** 
- **S. Hema Vaishnavi**
- **S.K Asifa** 
- **Samyuga**
- **Akshara Mikkilineni** 

**Institution:**   VNR Vignana Jyothi Institute of Engineering &Technology

**Department:** Electronics & Communication Engineering  

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
```
Copyright (c) 2024 Mahaveer Katighar, S. Hema Vaishnavi, S.K Asifa, Samyuga, Akshara Mikkilineni
```

## 🙏 Acknowledgments

- Firebase for cloud infrastructure
- Arduino & ESP32 community
- Medical professionals for requirements guidance
- Open-source library contributors

## ⚠️ Disclaimer

This is a prototype/educational project. For actual medical use:
- Obtain proper medical device certification
- Conduct thorough safety testing
- Comply with local medical device regulations
- Require qualified medical supervision

## 📞 Contact

For questions or collaboration:
- Open an issue on GitHub
- Email: mahaveerkatighar05@gmail.com

---

**Made with ❤️ for better healthcare**

**⭐ Star this repository if you find it helpful!**
```
