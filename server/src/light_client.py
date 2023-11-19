"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 17:53

class which handles light client
"""

import websockets.server as ws_server
import pydantic
import typing

from .client import Client
from .devices import connected_lights
from .datastore import specialized_file


class _LightCommandMessage(pydantic.BaseModel):
    type: typing.Literal["dist"]
    dist: int


_LightMessage = pydantic.TypeAdapter(_LightCommandMessage)


@specialized_file(base_path=["config", "lights"])
class _LightConfigFile(pydantic.BaseModel):
    ...


class LightClient(Client):
    @property
    def type_name(self) -> str:
        return "light"
    
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        super().__init__(socket)

        # set after identify
        if typing.TYPE_CHECKING:
            self._config = _LightConfigFile()
        else:
            self._config = None

    async def on_identified(self):
        # open config file
        self._config = _LightConfigFile([self.mac_hex])
        # register sensor
        connected_lights[self.mac_hex] = self
    
    async def on_message(self, msg: str):
        # try to validate the message
        msg = _LightMessage.validate_json(msg)
        # call handlers
        match msg:
            case _:
                ...

    async def on_disconnect(self):
        del connected_lights[self.mac_hex]
