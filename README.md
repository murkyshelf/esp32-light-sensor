# ESP32 Light Sensor with Fault Detection System

A real-time IoT fault detection system using ESP32 that monitors light circuits, detects various fault types, and reports them via HTTP API to a centralized server.

## 🌟 Features

- **🔍 Intelligent Fault Detection**: Detects short circuits, open circuits, overvoltage, undervoltage, and fluctuations
- **📡 HTTP API Integration**: Sends fault reports via POST requests with JSON payload
- **⚡ Real-time Monitoring**: Continuous voltage monitoring with 1-second sampling
- **🏷️ Light Identification**: Configurable light ID and device identification
- **🕐 Accurate Timestamps**: ISO 8601 formatted timestamps with NTP synchronization
- **🔐 Secure API**: API key authentication for server communication
- **📊 Multiple Fault Types**: Comprehensive fault classification system
- **🔄 Auto-reconnection**: Robust WiFi and HTTP error handling

## 📁 Project Structure

```
├── main/
│   ├── CMakeLists.txt          # Build configuration
│   ├── main.c                  # Original WebSocket version (legacy)
│   ├── main_fault_detector.c   # New fault detection firmware
│   └── idf_component.yml       # Component dependencies
├── fault-api-server/
│   ├── test_server.py          # Test API server (Flask)
│   ├── pyproject.toml          # Python project configuration
│   └── README.md               # API server documentation
├── websocket-server/           # Legacy WebSocket server (for reference)
├── build/                      # Compiled binaries and build files
├── CMakeLists.txt              # Main build configuration
└── sdkconfig                   # ESP-IDF configuration
```

## 🔧 Hardware Requirements

- **ESP32 Development Board** (tested with ESP32-D0WD-V3)
- **Light Sensor** (photoresistor, photodiode, or similar analog sensor)
- **10kΩ Resistor** (for voltage divider with photoresistor)
- **Breadboard and Jumper Wires**
- **USB Cable** for programming and power

## 🔌 Wiring Diagram

```
ESP32 GPIO34 (ADC1_CH6) ────── Light Sensor ────── 3.3V
                     │
                     └────── 10kΩ Resistor ────── GND
```

## 🚀 Quick Start

### 1. Prerequisites

- **ESP-IDF v5.5+** installed and configured
- **Python 3.8+** with websockets library
- **Git** for cloning the repository

### 2. Clone and Setup

```bash
git clone https://github.com/murkyshelf/esp32-light-sensor.git
cd esp32-light-sensor
```

### 3. Configure ESP32

Edit `main/main_fault_detector.c` to set your configuration:

```c
#define WIFI_SSID           "YourWiFiName"
#define WIFI_PASS           "YourWiFiPassword"
#define FAULT_API_URL       "http://YOUR_SERVER_IP:3000/api/fault/esp32"
#define API_KEY             "sexophobia69"
#define LIGHT_ID            "CB-420 light#2"
```

### 4. Start API Server

```bash
cd fault-api-server
pip install flask
python3 test_server.py
```

### 5. Build and Flash ESP32

```bash
# Setup ESP-IDF environment
source ~/esp/v5.5.1/esp-idf/export.sh

# Build the project
idf.py build

# Flash to ESP32
idf.py flash

# Monitor output
idf.py monitor
```

## � Fault Detection Types

| Fault Type | Voltage Range | Description |
|------------|---------------|-------------|
| **short_circuit** | < 0.3V | Short circuit detected in lighting circuit |
| **open_circuit** | > 3.2V | Open circuit or disconnected load |
| **overvoltage** | 3.1V - 3.2V | Voltage above normal operating range |
| **undervoltage** | 0.3V - 1.0V | Voltage below normal operating range |
| **voltage_fluctuation** | Any | Sudden voltage change > 0.5V |
| **normal** | 1.0V - 3.0V | Normal operation |

## 📊 Data Format

The ESP32 sends HTTP POST requests to the API endpoint:

```bash
POST /api/fault/esp32
Headers:
  Content-Type: application/json
  x-api-key: sexophobia69

Body:
{
  "fault-type": "short_circuit",
  "light-ID": "CB-420 light#2",
  "timestamp": "2025-09-07T10:37:27Z"
}
```

Server response:
```json
{
  "status": "success",
  "message": "Fault report received successfully",
  "report_id": 123,
  "received_at": "2025-09-07T10:37:28Z"
}
```

## 🛠️ Configuration

### ESP32 Configuration (`main/main_fault_detector.c`)

| Parameter | Description | Default |
|-----------|-------------|---------|
| `WIFI_SSID` | WiFi network name | "Ce4" |
| `WIFI_PASS` | WiFi password | "p4msujax" |
| `FAULT_API_URL` | API endpoint URL | "http://10.230.25.216:3000/api/fault/esp32" |
| `API_KEY` | API authentication key | "sexophobia69" |
| `LIGHT_ID` | Unique light identifier | "CB-420 light#2" |
| `DEVICE_ID` | ESP32 device identifier | "ESP32-001" |

### Fault Detection Thresholds

| Parameter | Description | Default |
|-----------|-------------|---------|
| `NORMAL_MIN_VOLTAGE` | Minimum normal voltage | 1.0V |
| `NORMAL_MAX_VOLTAGE` | Maximum normal voltage | 3.0V |
| `SHORT_CIRCUIT_VOLTAGE` | Short circuit threshold | < 0.3V |
| `OPEN_CIRCUIT_VOLTAGE` | Open circuit threshold | > 3.2V |
| `OVERVOLTAGE_THRESHOLD` | Overvoltage detection | > 3.1V |

### API Server Configuration (`fault-api-server/test_server.py`)

| Parameter | Description | Default |
|-----------|-------------|---------|
| `host` | Server listening address | "0.0.0.0" |
| `port` | API server port | 3000 |
| `api_key` | Required API key | "sexophobia69" |

## 🔍 Monitoring and Debugging

### ESP32 Serial Monitor
```bash
idf.py monitor
```

### WebSocket Server Logs
The enhanced server provides real-time logs with:
- 🔗 Connection status
- 📊 Formatted data reception
- ⚡ Voltage value highlights
- 📤 Response confirmations
- 🔌 Disconnection events

### Network Diagnostics
```bash
# Check if server is listening
ss -tlnp | grep 8080

# Monitor network traffic
tcpdump -i any port 8080
```

## 🐛 Troubleshooting

### Common Issues

**ESP32 not connecting to WiFi:**
- Verify SSID and password in `main.c`
- Check WiFi signal strength
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)

**WebSocket connection failed:**
- Verify server IP address and port
- Check firewall settings
- Ensure server is running before ESP32 attempts connection

**Permission denied on serial port:**
```bash
sudo usermod -a -G dialout $USER
# Logout and login again
```

**Port already in use:**
```bash
pkill -f server.py
# Wait a few seconds, then restart server
```

## 📈 Extending the Project

### Adding More Sensors
- Temperature/Humidity (DHT22)
- Motion detection (PIR sensor)
- Air quality (MQ-135)

### Data Storage
- SQLite database integration
- CSV file logging
- Cloud database (Firebase, InfluxDB)

### Web Dashboard
- Real-time charts with Chart.js
- Historical data visualization
- Mobile-responsive interface

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature-name`
3. Commit changes: `git commit -am 'Add feature'`
4. Push to branch: `git push origin feature-name`
5. Submit a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**murkyshelf** - [GitHub Profile](https://github.com/murkyshelf)

## 🙏 Acknowledgments

- Espressif Systems for ESP-IDF framework
- Python websockets library contributors
- ESP32 community for documentation and examples
