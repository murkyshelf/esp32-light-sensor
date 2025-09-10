import asyncio
import websockets
import json
import datetime

async def echo(websocket, path):
    client_ip = websocket.remote_address[0]
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] 🔗 Client connected from {client_ip}")
    
    try:
        async for message in websocket:
            timestamp = datetime.datetime.now().strftime("%H:%M:%S")
            
            # Try to parse as JSON for prettier output
            try:
                data = json.loads(message)
                print(f"[{timestamp}] 📊 ESP32 Data: {json.dumps(data, indent=2)}")
                
                # If it's voltage data, show it nicely
                if 'voltage' in data:
                    voltage = data['voltage']
                    print(f"[{timestamp}] ⚡ Voltage Reading: {voltage:.6f}V")
                    
            except json.JSONDecodeError:
                print(f"[{timestamp}] 📨 Raw message: {message}")
            
            # Echo the message back to the client
            echo_response = f"Echo: {message}"
            await websocket.send(echo_response)
            print(f"[{timestamp}] 📤 Sent echo response to ESP32")
            
    except websockets.exceptions.ConnectionClosed:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] 🔌 Client {client_ip} disconnected")
    except Exception as e:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        print(f"[{timestamp}] ❌ Error: {e}")

async def main():
    timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    print(f"[{timestamp}] 🚀 Starting WebSocket server on ws://0.0.0.0:8080")
    print(f"[{timestamp}] 👀 All communication will be logged here...")
    print("-" * 60)
    
    async with websockets.serve(echo, "0.0.0.0", 8080):  # Using port 8080 to match ESP32
        await asyncio.Future()  # Run forever

if __name__ == "__main__":
    asyncio.run(main())
