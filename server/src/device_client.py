"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
08.11.23, 16:13

class which handles a sensor client
"""

import websockets.server as ws_server

from .client import Client
from .device import Device
from .sensor_device import SensorDevice
from .light_device import LightDevice
from .messages import *


class DeviceClient(Client):
    @property
    def type_name(self) -> str:
        return "device client"
    
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        super().__init__(socket)

        self._subdevices: list[Device] = []
    
    def __del__(self):
        print("client deleted")
    
    async def on_identified(self):
        # create all the 
        print(f"Initializing {self._nr_sensors} sensor(s) and {self._nr_lights} lights(s)")
        sub_nr: int = 0
        for _ in range(self._nr_sensors):
            self._subdevices.append(SensorDevice(self.mac_hex, sub_nr, self))
            sub_nr += 1
        for _ in range(self._nr_lights):
            self._subdevices.append(LightDevice(self.mac_hex, sub_nr, self))
            sub_nr += 1
    
    async def on_message(self, msg_in: str):
        # try to validate the message
        msg = DeviceIncomingMessage.validate_json(msg_in)
        target = self._subdevices[msg.sub]
        # call handlers
        match msg:
            case SensorDistanceMessage():
                assert isinstance(target, SensorDevice)
                await target.on_distance_update(msg.dist)
            case LightAnimationDoneMessage():
                assert isinstance(target, LightDevice)
                await target.on_animation_done(msg.id)
            case _:
                print("message not implemented jet")

    async def on_disconnect(self):
        for device in self._subdevices:
            await device.on_disconnect()
    
    async def send_message(self, message: BaseMessage):
        """
        encodes and sends a message to a client.
        This is used by device classes to send their specific messages.
        """
        await self._socket.send(message.model_dump_json())
