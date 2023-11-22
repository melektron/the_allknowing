"""
ELEKTRON © 2023 - now
Written by melektron & Nilusink
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
class LightBlitzMessage(BaseMessage):
    type: typing.Literal["blitz"] = "blitz"
    dur: int    # duration in ms


class LightWaveMessage(BaseMessage):
    type: typing.Literal["wave"] = "wave"
    dur: int    # duration in ms


class LightAvoidMessage(BaseMessage):
    type: typing.Literal["avoid"] = "avoid"
    speed: int  # retraction speed from 0 to 100


class LightBlinkMessage(BaseMessage):
    type: typing.Literal["blink"] = "blink"
    dur: int    # duration per blink in ms
    n: int      # number of blinks


class LightRainbowMessage(BaseMessage):
    type: typing.Literal["rainbow"]
    dur: int    # duration in ms
