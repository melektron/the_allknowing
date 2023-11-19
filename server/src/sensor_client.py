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


class _SensorDistanceMessage(pydantic.BaseModel):
    type: typing.Literal["dist"]
    dist: int


_SensorMessage = pydantic.TypeAdapter(_SensorDistanceMessage)


@specialized_file(base_path=["config", "sensors"])
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


class SensorClient(Client):
    @property
    def type_name(self) -> str:
        return "sensor"
    
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        super().__init__(socket)

        # set after identify
        if typing.TYPE_CHECKING:
            self._config = _SensorConfigFile()
        else:
            self._config = None

        # trigger functionality
        self._is_triggered = False
        # distance functionality
        self._distance = 0
    
    async def on_identified(self):
        # open config file
        self._config = _SensorConfigFile([self.mac_hex])
        # register sensor
        connected_sensors[self.mac_hex] = self
    
    async def on_message(self, msg: str):
        # try to validate the message
        msg = _SensorMessage.validate_json(msg)
        # call handlers
        match msg:
            case _SensorDistanceMessage():
                await self._handle_distance_message(msg)

    async def on_disconnect(self):
        del connected_sensors[self.mac_hex]

    async def _handle_distance_message(self, msg: _SensorDistanceMessage):
        if not self._has_identified:
            return
        
        self._distance = msg.dist

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
            print(f"sensor {self.mac_hex}: trigger on")

    async def _send_trigger_off_event(self):
        if self._config.uses_trigger_off:
            print(f"sensor {self.mac_hex}: trigger off")

    async def _send_distance_changed_event(self):
        if self._config.uses_distance_change:
            print(f"sensor {self.mac_hex}: distance changed to {self._distance}cm")