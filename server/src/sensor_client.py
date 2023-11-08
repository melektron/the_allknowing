"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
08.11.23, 16:13

class which handles a sensor client
"""

import websockets.server as ws_server
import websockets.exceptions as ws_ex

class SensorClient:
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        print("sensor connected")
        self._socket = socket
    
    async def process_client(self):
        try:
            async for message in self._socket:
                await self._socket.send(message)
                print(f"got sensor message: {message}")
        except ws_ex.ConnectionClosed:
            print("sensor disconnected")

