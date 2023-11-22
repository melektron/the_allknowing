"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 20:50

specialization for sensor devices
"""

import pydantic
import typing
from collections import deque

from .device import Device
from .datastore import specialized_file
from .devices import connected_sensors
from .actions import AnyAction, run_actions

if typing.TYPE_CHECKING:
    from .device_client import DeviceClient


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
    trigger_samples: int = 3    # how many samples should be averaged (running average) for trigger processing

    # actions to perform on events
    on_trigger_on: list[AnyAction] = []
    on_trigger_off: list[AnyAction] = []
    on_distance_change: list[AnyAction] = []


class SensorDevice(Device):
    @property
    def type_name(self) -> str:
        return "sensor"
    
    def __init__(self, client_mac: str, subdevice_id: int, client: "DeviceClient") -> None:
        super().__init__(client_mac, subdevice_id, client)

        # config file for the device
        self._config = _SensorConfigFile(self.config_path)

        # trigger functionality
        self._is_triggered = False
        self._running_avg_samples: deque[int] = deque(maxlen=self._config.trigger_samples)
        self._running_avg_distance: int = 0
        # current distance for distance functionality
        self._distance = 0

        connected_sensors[self.id] = self
    
    async def on_disconnect(self) -> None:
        del connected_sensors[self.id]
    
    async def on_distance_update(self, distance: int) -> None:
        # save distance
        self._distance = distance

        # update running average:
        self._running_avg_samples.append(distance)
        self._running_avg_distance = sum(self._running_avg_samples) / len(self._running_avg_samples)

        # create level trigger events
        if self._is_triggered:
            if self._config.trigger_direction == "above":
                if self._running_avg_distance < self._config.trigger_distance - self._config.trigger_hysteresis:
                    self._is_triggered = False
                    await self._send_trigger_off_event()
            elif self._config.trigger_direction == "below":
                if self._running_avg_distance > self._config.trigger_distance + self._config.trigger_hysteresis:
                    self._is_triggered = False
                    await self._send_trigger_off_event()
        else:
            if self._config.trigger_direction == "above":
                if self._running_avg_distance > self._config.trigger_distance:
                    self._is_triggered = True
                    await self._send_trigger_on_event()
            elif self._config.trigger_direction == "below":
                if self._running_avg_distance < self._config.trigger_distance:
                    self._is_triggered = True
                    await self._send_trigger_on_event()
        
        # create distance changed event
        await self._send_distance_changed_event()
    
    async def _send_trigger_on_event(self) -> None:
        if not self._config.uses_trigger_on:
            return
        print(f"sensor {self.id}: trigger on")
        await run_actions(self._config.on_trigger_on, self._running_avg_distance)

    async def _send_trigger_off_event(self) -> None:
        if not self._config.uses_trigger_off:
            return
        print(f"sensor {self.id}: trigger off")
        await run_actions(self._config.on_trigger_off, self._running_avg_distance)

    async def _send_distance_changed_event(self) -> None:
        if not self._config.uses_distance_change:
            return
        print(f"sensor {self.id}: distance changed to {self._distance}cm")
        await run_actions(self._config.on_distance_change, self._distance)