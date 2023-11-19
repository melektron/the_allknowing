"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 17:52

exports lists of connected devices
"""

import typing

if typing.TYPE_CHECKING:
    from .sensor_device import SensorDevice
    from .light_device import LightDevice

connected_sensors: dict[str, "SensorDevice"] = dict()
connected_lights: dict[str, "LightDevice"] = dict()
