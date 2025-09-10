# WebSocket Server for ESP32 Light Sensor

Python WebSocket server to receive and log real-time sensor data from ESP32 devices.

## 📁 Files

- **`server.py`** - Basic WebSocket echo server
- **`server_with_logs.py`** - Enhanced server with detailed logging and formatting
- **`pyproject.toml`** - Python project configuration with dependencies

## 🚀 Quick Start

### Install Dependencies

```bash
# Using pip
pip install websockets

# Or using the project file
pip install -e .
```

### Run the Enhanced Server

```bash
python3 server_with_logs.py
```

The server will start on `ws://0.0.0.0:8080` and display:
```
[2025-09-11 01:23:21] 🚀 Starting WebSocket server on ws://0.0.0.0:8080
[2025-09-11 01:23:21] 👀 All communication will be logged here...
------------------------------------------------------------
```

## 📊 Server Features

### Enhanced Logging (`server_with_logs.py`)

- **🔗 Connection Tracking**: Shows when ESP32 devices connect/disconnect
- **📊 JSON Formatting**: Pretty-prints received sensor data
- **⚡ Voltage Highlighting**: Special formatting for voltage readings
- **📤 Echo Responses**: Confirms data received with echo messages
- **🕐 Timestamps**: All events are timestamped

### Example Output

```
[01:23:45] 🔗 Client connected from 10.230.25.115
[01:23:50] 📊 ESP32 Data: {
  "voltage": 2.845123
}
[01:23:50] ⚡ Voltage Reading: 2.845123V
[01:23:50] 📤 Sent echo response to ESP32
[01:23:55] 📊 ESP32 Data: {
  "voltage": 1.234567
}
[01:23:55] ⚡ Voltage Reading: 1.234567V
[01:23:55] 📤 Sent echo response to ESP32
```

## 🛠️ Configuration

### Server Settings

| Parameter | Description | Default |
|-----------|-------------|---------|
| `host` | Listening address | `"0.0.0.0"` (all interfaces) |
| `port` | WebSocket port | `8080` |

### Customization

To modify the server behavior, edit `server_with_logs.py`:

```python
# Change port
async with websockets.serve(echo, "0.0.0.0", 8081):  # Port 8081

# Change host (localhost only)
async with websockets.serve(echo, "127.0.0.1", 8080):  # Localhost only
```

## 🔍 Monitoring

### Check Server Status

```bash
# Verify server is listening
ss -tlnp | grep 8080

# View all network connections
netstat -tulpn | grep 8080
```

### Stop Server

- **Keyboard**: Press `Ctrl+C`
- **Command line**: `pkill -f server_with_logs.py`

## 🐛 Troubleshooting

### Port Already in Use

```bash
# Kill existing server processes
pkill -f server.py
pkill -f server_with_logs.py

# Wait a few seconds, then restart
python3 server_with_logs.py
```

### Connection Issues

1. **Firewall**: Ensure port 8080 is open
2. **Network**: Server binds to `0.0.0.0` to accept connections from other devices
3. **ESP32 Configuration**: Verify ESP32 has correct server IP address

### Permission Denied

```bash
# If running on privileged port (< 1024)
sudo python3 server_with_logs.py

# Or use a higher port number (recommended)
# Edit the script to use port 8080 or higher
```

## 📈 Extensions

### Data Storage

Add database logging:

```python
import sqlite3

# In the echo function
def save_to_db(voltage_data):
    conn = sqlite3.connect('sensor_data.db')
    cursor = conn.cursor()
    cursor.execute('''
        INSERT INTO readings (timestamp, voltage) 
        VALUES (datetime('now'), ?)
    ''', (voltage_data['voltage'],))
    conn.commit()
    conn.close()
```

### Web Dashboard

Serve a web interface:

```python
import http.server
import socketserver
import threading

# Start HTTP server for web interface
def start_web_server():
    handler = http.server.SimpleHTTPRequestHandler
    with socketserver.TCPServer(("", 8000), handler) as httpd:
        httpd.serve_forever()

# Run in background
threading.Thread(target=start_web_server, daemon=True).start()
```

### Multiple Clients

Track multiple ESP32 devices:

```python
connected_clients = set()

async def echo(websocket, path):
    connected_clients.add(websocket)
    try:
        # Handle messages
        pass
    finally:
        connected_clients.remove(websocket)
```

## 🤝 Contributing

This server is part of the ESP32 Light Sensor project. See the main [README.md](../README.md) for contribution guidelines.
