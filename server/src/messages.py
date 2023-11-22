"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 21:56

all message data models for the device client
"""

import typing
import pydantic


class BaseMessage(pydantic.BaseModel):
    type: str
    sub: int

class SensorDistanceMessage(BaseMessage):
    type: typing.Literal["dist"]
    dist: int


class LightAnimationDoneMessage(BaseMessage):
    type: typing.Literal["ad"]
    id: int


DeviceIncomingMessage = pydantic.TypeAdapter(SensorDistanceMessage | LightAnimationDoneMessage)


class LightSetBrightnessMessage(BaseMessage):
    type: typing.Literal["br"] = "br"
    br: int

class LightSetStaticColorMessage(BaseMessage):
    type: typing.Literal["col"] = "col"
    r: int
    g: int
    b: int

class LightBlitzMessage(BaseMessage):
    type: typing.Literal["blitz"] = "blitz"
    dur: int    # duration in ms
