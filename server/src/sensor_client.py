"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
08.11.23, 16:13

class which handles a sensor client
"""

import websockets.server as ws_server
import websockets.exceptions as ws_ex
import pydantic
import typing


class SensorIdentifyMessage(pydantic.BaseModel):
    type: typing.Literal["mac"] 
    mac: int

class SensorDistancMessage(pydantic.BaseModel):
    type: typing.Literal["dist"] 
    dist: int

SensorMessage = pydantic.TypeAdapter[SensorIdentifyMessage | SensorDistancMessage]

class SensorClient:
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        print("sensor connected")
        self._socket = socket
    
    async def process_client(self):
        try:
            async for message in self._socket:
                await self._socket.send(message)
                try:
                    msg = SensorMessage.validate_json(message)
                except pydantic.ValidationError:
                    print("got invalid message")
                print(f"got sensor message: {msg}")
        except ws_ex.ConnectionClosed:
            print("sensor disconnected")

