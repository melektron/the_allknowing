"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 20:35

class representing one logical device.
A client may have multiple logical devices.
"""

import websockets.server as ws_server
import websockets.exceptions as ws_ex
import pydantic
import typing
import abc



class Device(abc.ABC):
    def __init__(self, client_mac: str, subdevice_id: int) -> None:
        self._id: str = client_mac + f".{subdevice_id}"

    @property
    def id(self) -> str:
        """
        id of the device (consists of client mac and subdevice id)
        example:

        5452fc5d8634.1
        """
        return self._id
    
    @property
    def config_path(self) -> list[str]:
        return ["config", self.type_name, self._id]

    @abc.abstractproperty
    def type_name(self) -> str:
        """
        type name of the device, e.g. sensor or light
        """
        ...
    
    @abc.abstractmethod
    async def on_disconnect(self):
        """
        called when the client the device is associated with disconnects.
        Any cleanup can be done here.
        """
        ...