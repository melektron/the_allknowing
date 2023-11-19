"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 21:34

specialization for light devices
"""

import pydantic
import typing

from .device import Device
from .devices import connected_lights
from .messages import *

if typing.TYPE_CHECKING:
    from .device_client import DeviceClient


class LightDevice(Device):
    @property
    def type_name(self) -> str:
        return "light"
    
    def __init__(self, client_mac: str, subdevice_id: int, client: "DeviceClient") -> None:
        super().__init__(client_mac, subdevice_id, client)

        self._animation_id_counter: int = 0
        self._running_animations: dict[int, typing.Any] = {}

        connected_lights[self.id] = self
    
    async def on_disconnect(self):
        del connected_lights[self.id]
    
    async def on_animation_done(self, anim_id: int):
        del self._running_animations[anim_id]
    
    async def animate_blitz(self, duration: int):
        await self._client.send_message(LightBlitzMessage(
            sub=self._subdevice_id,
            id=self._animation_id_counter,
            dur=duration
        ))
        self._animation_id_counter += 1
