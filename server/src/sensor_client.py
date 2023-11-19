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
import asyncio

from .devices import connected_sensors
from .datastore import specialized_file


class _SensorIdentifyMessage(pydantic.BaseModel):
    type: typing.Literal["mac"]
    mac: int


class _SensorDistancMessage(pydantic.BaseModel):
    type: typing.Literal["dist"]
    dist: int


_SensorMessage = pydantic.TypeAdapter(_SensorIdentifyMessage | _SensorDistancMessage)


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



class SensorClient:
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        print("sensor connected")
        self._socket = socket

        # after identification, the sensor will be properly initialized and these
        # members set to a reasonable value
        self._has_identified = False
        self._mac = 0
        if typing.TYPE_CHECKING:
            self._config = _SensorConfigFile()
        else:
            self._config = None

        # trigger functionality
        self._is_triggered = False
        # distance functionality
        self._distance = 0
    
    @property
    def mac_hex(self) -> str:
        return hex(self._mac)

    async def process_client(self):
        try:
            # wait for a message
            async for message in self._socket:
                try:
                    # try to validate it
                    msg = _SensorMessage.validate_json(message)
                    # call handlers
                    match msg:
                        case _SensorIdentifyMessage():
                            await self._handle_identify_message(msg)
                        case _SensorDistancMessage():
                            await self._handle_distance_message(msg)

                except pydantic.ValidationError:
                    print(f"got invalid message from sensor {self.mac_hex}")

        except ws_ex.ConnectionClosed:
            self._handle_disconnect()

    def _handle_disconnect(self):
        print(f"sensor {self.mac_hex if self._has_identified else "(unidentified)"} disconnected")
        connected_sensors.discard(self)

    async def _handle_identify_message(self, msg: _SensorIdentifyMessage):
        self._has_identified = True
        self._mac = msg.mac
        # open config file
        self._config = _SensorConfigFile([hex(self._mac)])

        print(f"sensor {self.mac_hex} has identified itself and is ready")
        connected_sensors.add(self)

    async def _handle_distance_message(self, msg: _SensorDistancMessage):
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