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
import enum

from .client import Client
from .devices import connected_lights
from .datastore import specialized_file



class _LightAnimationDoneMessage(pydantic.BaseModel):
    type: typing.Literal["ad"]
    id: int


_LightIncomingMessage = pydantic.TypeAdapter(_LightAnimationDoneMessage)


class _LightSetBrightnessMessage(pydantic.BaseModel):
    type: typing.Literal["br"] = "br"
    br: bool


class _LightStartAnimationMessage(pydantic.BaseModel):
    id: int


class _LightBlitzMessage(_LightStartAnimationMessage):
    type: typing.Literal["blitz"] = "blitz"
    dur: int    # duration in ms


class LightClient(Client):
    @property
    def type_name(self) -> str:
        return "light client"
    
    def __init__(self, socket: ws_server.WebSocketServerProtocol) -> None:
        super().__init__(socket)


    async def on_identified(self):
        ...
    
    async def on_message(self, msg: str):
        # try to validate the message
        msg = _LightIncomingMessage.validate_json(msg)
        # call handlers
        match msg:
            case _:
                ...

    async def on_disconnect(self):
        ...
