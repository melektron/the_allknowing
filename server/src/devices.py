"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 17:52

exports lists of connected devices
"""

import typing

if typing.TYPE_CHECKING:
    from .sensor_client import SensorClient
    from .light_client import LightClient

connected_sensors: dict[str, "SensorClient"] = dict()
connected_lights: dict[str, "LightClient"] = dict()