import asyncio
import websockets

async def listen():
    uri = "ws://localhost:8080"  # Change to your WebSocket server URI
    async with websockets.connect(uri) as websocket:
        while True:
            message = await websocket.recv()
            print(f"Received: {message}")

asyncio.run(listen())