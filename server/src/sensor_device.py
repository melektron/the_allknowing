"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 20:50

specialization for sensor devices
"""

import pydantic
import typing

from .device import Device
from .datastore import specialized_file
from .devices import connected_sensors

@specialized_file
class _SensorConfigFile(pydantic.BaseModel):
    # what trigger events should be used
    uses_trigger_on: bool = False
    uses_trigger_off: bool = False

    # whether distance change events are used
    uses_distance_change: bool = False

    # trigger value configuration
    trigger_distance: int = 50  # cm
    trigger_direction: typing.Literal["above", "below"] = "below"
    trigger_hysteresis: int = 10    # cm


class SensorDevice(Device):
    @property
    def type_name(self) -> str:
        return "sensor"
    
    def __init__(self, client_mac: str, subdevice_id: int) -> None:
        super().__init__(client_mac, subdevice_id)

        # config file for the device
        self._config = _SensorConfigFile(self.config_path)

        # trigger functionality
        self._is_triggered = False
        # distance functionality
        self._distance = 0

        connected_sensors[self.id] = self
    
    async def on_disconnect(self):
        del connected_sensors[self.id]
    
    async def on_distance_update(self, distance: int):
        self._distance = distance

        # create level trigger events
        if self._is_triggered:
            if self._config.trigger_direction == "above":
                if self._distance < self._config.trigger_distance - self._config.trigger_hysteresis:
                    self._is_triggered = False
                    await self._send_trigger_off_event()
            elif self._config.trigger_direction == "below":
                if self._distance > self._config.trigger_distance + self._config.trigger_hysteresis:
                    self._is_triggered = False
                    await self._send_trigger_off_event()
        else:
            if self._config.trigger_direction == "above":
                if self._distance > self._config.trigger_distance:
                    self._is_triggered = True
                    await self._send_trigger_on_event()
            elif self._config.trigger_direction == "below":
                if self._distance < self._config.trigger_distance:
                    self._is_triggered = True
                    await self._send_trigger_on_event()
        
        # create distance changed event
        await self._send_distance_changed_event()
    
    async def _send_trigger_on_event(self):
        if self._config.uses_trigger_on:
            print(f"sensor {self.id}: trigger on")

    async def _send_trigger_off_event(self):
        if self._config.uses_trigger_off:
            print(f"sensor {self.id}: trigger off")

    async def _send_distance_changed_event(self):
        if self._config.uses_distance_change:
            print(f"sensor {self.id}: distance changed to {self._distance}cm")