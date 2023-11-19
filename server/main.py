"""

"""

import sys
import asyncio

import websockets.server as ws_server
import websockets.exceptions as ws_ex

from src.sensor_client import SensorClient
from src.light_client import LightClient
from src.devices import connected_lights, connected_sensors

async def client_receiver(socket: ws_server.WebSocketServerProtocol, path: str):
    print(f"{path=}")
    match path:
        case "/":
            print("test socket connected")
            try:
                async for message in socket:
                    await socket.send(message)
                    print(f"got echo test message: {message}")
            except ws_ex.ConnectionClosed:
                print("test socket disconnected")
        
        case "/sensor":
            sensor = SensorClient(socket=socket)
            await sensor.process_client()
        case "/light":
            light = LightClient(socket=socket)
            await light.process_client()


async def main() -> int:
    async with ws_server.serve(
        client_receiver, 
        "0.0.0.0", 
        80,
        ping_interval=5,
        ping_timeout=5
    ):
        while True:
            #print(f"sensors: {", ".join(connected_sensors)}")
            await asyncio.sleep(3)


if __name__ == "__main__":
    try:
        sys.exit(asyncio.run(main()))
    except KeyboardInterrupt:
        print()
        sys.exit(1)