"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 19:50

class containing base functions of all clients
"""

import websockets.server as ws_server
import websockets.exceptions as ws_ex
import pydantic
import typing
import abc


class _ClientIdentifyMessage(pydantic.BaseModel):
    type: typing.Literal["id"]
    mac: int        # MAC address of device
    sensors: int    # nr of sensors
    lights: int     # nr of lights


class Client(abc.ABC):
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        self._socket = socket

        # after identification, the sensor will be properly initialized and these
        # members set to a reasonable value
        self._has_identified = False
        self._mac = 0
        self._nr_sensors = 0
        self._nr_lights = 0
    
    @property
    def _nr_subdevices(self) -> int:
        self._nr_sensors + self._nr_lights

    @property
    def mac_hex(self) -> str:
        """
        hex encoded mac address of the client
        """
        return hex(self._mac)[2:]

    async def process_client(self):
        """
        main processing loop function handling the client asynchronously
        """
        try:
            # wait for a message
            async for message in self._socket:
                # wait for initial identify message
                if not self._has_identified:
                    try:
                        id_msg = _ClientIdentifyMessage.model_validate_json(message)
                        await self._handle_identify_message(id_msg)
                    except pydantic.ValidationError:
                        print(f"got invalid identification message from {self.type_name} {self.mac_hex}")

                # after identification pass messages to handler
                else:
                    try:
                        await self.on_message(str(message))
                    except pydantic.ValidationError:
                        print(f"got invalid message from {self.type_name} {self.mac_hex}")

        except ws_ex.ConnectionClosed:
            await self._handle_disconnect()

    async def _handle_disconnect(self):
        print(f"{self.type_name} {self.mac_hex if self._has_identified else "(unidentified)"} disconnected")
        await self.on_disconnect()

    async def _handle_identify_message(self, msg: _ClientIdentifyMessage):
        """
        handles the identification message
        """
        self._has_identified = True
        self._mac = msg.mac
        self._nr_sensors = msg.sensors
        self._nr_lights = msg.lights
        print(f"{self.type_name} {self.mac_hex} has identified itself and is ready")
        await self.on_identified()
    
    @abc.abstractproperty
    def type_name(self) -> str:
        """
        type name of the client, e.g. sensor or light
        """
        ...
    
    @abc.abstractmethod
    async def on_identified(self):
        """
        called once the client has successfully identified itself
        """
        ...

    @abc.abstractmethod
    async def on_message(self, msg: str):
        """
        called whenever a client message arrives (after client has identified)
        """
        ...
    
    @abc.abstractmethod
    async def on_disconnect(self):
        """
        called when the client has disconnected to perform some cleanup
        """
        ...