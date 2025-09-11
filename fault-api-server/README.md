# ESP32 Fault Detection API Server

Test server that simulates your fault detection API endpoint.

## Quick Start

```bash
# Install dependencies
pip install flask requests

# Start the server
python3 test_server.py
```

The server will start on `http://0.0.0.0:3000`

## API Endpoints

### POST /api/fault/esp32
Receives fault data from ESP32 devices.

**Headers:**
- `Content-Type: application/json`
- `x-api-key: sexophobia69`

**Body:**
```json
{
    "fault-type": "short_circuit",
    "light-ID": "CB-420 light#2", 
    "timestamp": "2025-09-07T10:37:27Z"
}
```

### GET /api/fault/reports
View recent fault reports (for debugging).

### GET /api/health
Health check endpoint.

## Testing

Test the API with curl:
```bash
curl -X POST http://localhost:3000/api/fault/esp32 \
  -H "Content-Type: application/json" \
  -H "x-api-key: sexophobia69" \
  -d '{"fault-type":"short_circuit","light-ID":"CB-420 light#2","timestamp":"2025-09-07T10:37:27Z"}'
```
