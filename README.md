# ESP32 Light Sensor with WebSocket Communication

A real-time IoT light sensor project using ESP32 that reads analog light values and transmits them via WebSocket to a Python server for monitoring and logging.

## 🌟 Features

- **Real-time Light Sensing**: Reads analog values from GPIO34 (ADC1_CHANNEL_6)
- **WiFi Connectivity**: Connects to WiFi network automatically
- **WebSocket Communication**: Streams sensor data in JSON format every 5 seconds
- **Python Server**: Receives and logs data with enhanced formatting
- **12-bit ADC Resolution**: High precision voltage readings (0-3.3V range)
- **Auto-reconnection**: Robust connection handling for both WiFi and WebSocket

## 📁 Project Structure

```
├── main/
│   ├── CMakeLists.txt          # Build configuration
│   ├── main.c                  # ESP32 firmware source code
│   └── idf_component.yml       # Component dependencies
├── websocket-server/
│   ├── server.py               # Basic WebSocket server
│   ├── server_with_logs.py     # Enhanced server with detailed logging
│   ├── pyproject.toml          # Python project configuration
│   └── README.md               # Server documentation
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
ESP32 GPIO34 (ADC1_CH6) ────── Solar cell ────── GND
                     
                     
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

Edit `main/main.c` to set your WiFi credentials and server IP:

```c
#define WIFI_SSID      "YourWiFiName"
#define WIFI_PASS      "YourWiFiPassword"
#define WEBSOCKET_URI  "ws://YOUR_SERVER_IP:8080"
```

### 4. Build and Flash ESP32

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

### 5. Start WebSocket Server

```bash
cd websocket-server
python3 server_with_logs.py
```

## 📊 Data Format

The ESP32 sends JSON data every 5 seconds:

```json
{
  "voltage": 2.845123
}
```

Server logs display:
```
[01:23:45] 🔗 Client connected from 192.168.1.100
[01:23:50] 📊 ESP32 Data: {
  "voltage": 2.845123
}
[01:23:50] ⚡ Voltage Reading: 2.845123V
[01:23:50] 📤 Sent echo response to ESP32
```

## 🛠️ Configuration

### ESP32 Configuration (`main/main.c`)

| Parameter | Description | Default |
|-----------|-------------|---------|
| `WIFI_SSID` | WiFi network name | "ssid" |
| `WIFI_PASS` | WiFi password | "password" |
| `WEBSOCKET_URI` | WebSocket server address | "ws://\<address\>:8080" |
| `ADC_CHANNEL` | ADC input pin | ADC1_CHANNEL_6 (GPIO34) |

### Server Configuration (`websocket-server/server_with_logs.py`)

| Parameter | Description | Default |
|-----------|-------------|---------|
| `host` | Server listening address | "0.0.0.0" |
| `port` | WebSocket port | 8080 |

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
