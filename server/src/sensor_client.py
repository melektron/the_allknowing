"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
08.11.23, 16:13

class which handles a sensor client
"""

import websockets.server as ws_server
import pydantic
import typing

from .client import Client
from .devices import connected_sensors
from .datastore import specialized_file
from .sensor_device import SensorDevice


class _SensorDistanceMessage(pydantic.BaseModel):
    type: typing.Literal["dist"]
    sub: int
    dist: int


_SensorIncomingMessage = pydantic.TypeAdapter(_SensorDistanceMessage)



class SensorClient(Client):
    @property
    def type_name(self) -> str:
        return "sensor client"
    
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        super().__init__(socket)

        self._subdevices: list[SensorDevice] = []
    
    async def on_identified(self):
        # create all the subdevices
        for sub_nr in range(self._nr_subdevices):
            self._subdevices.append(SensorDevice(self.mac_hex, sub_nr))
    
    async def on_message(self, msg: str):
        # try to validate the message
        msg = _SensorIncomingMessage.validate_json(msg)
        # call handlers
        match msg:
            case _SensorDistanceMessage():
                await self._handle_distance_message(msg)

    async def on_disconnect(self):
        for device in self._subdevices:
            await device.on_disconnect()

    async def _handle_distance_message(self, msg: _SensorDistanceMessage):
        await self._subdevices[msg.sub].on_distance_update(msg.dist)