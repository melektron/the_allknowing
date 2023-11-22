"""
ELEKTRON © 2023 - now
Written by melektron & Nilusink
www.elektron.work
19.11.23, 21:34

specialization for light devices
"""

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

        #self._animation_id_counter: int = 0
        #self._running_animations: dict[int, typing.Any] = {}
        #self._animation_id_counter: int = 0
        #self._running_animations: dict[int, typing.Any] = {}

        connected_lights[self.id] = self
    
    async def on_disconnect(self) -> None:
        del connected_lights[self.id]
    
    #async def on_animation_done(self, anim_id: int):
    #    del self._running_animations[anim_id]

    async def set_brightness(self, br: int) -> None:
        """
        br 0..255
        """
        await self._client.send_message(LightSetBrightnessMessage(
            sub=self._subdevice_id,
            br=br
        ))

    async def set_static_color(self, color: tuple[int, int, int]) -> None:
        await self._client.send_message(LightSetStaticColorMessage(
            sub=self._subdevice_id,
            r=color[0],
            g=color[1],
            b=color[2],
        ))
    
    async def animate_blitz(self, duration: int) -> None:
        await self._client.send_message(LightBlitzMessage(
            sub=self._subdevice_id,
            dur=duration
        ))
        #self._animation_id_counter += 1

    async def animate_wave(self, duration: int) -> None:
        await self._client.send_message(LightWaveMessage(
            sub=self._subdevice_id,
            dur=duration
        ))

    async def animate_avoid(self, speed: int) -> None:
        await self._client.send_message(LightWaveMessage(
            sub=self._subdevice_id,
            speed=speed
        ))

    async def animate_blink(self, duration: int, n_blinks: int) -> None:
        await self._client.send_message(LightBlinkMessage(
            sub=self._subdevice_id,
            dur=duration,
            n=n_blinks
        ))

    async def animate_rainbow(self, duration: int) -> None:
        await self._client.send_message(LightRainbowMessage(
            sub=self._subdevice_id,
            dur=duration
        ))
