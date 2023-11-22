"""
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
19.11.23, 22:17

Actions that can be executed.
"""

import typing
import pydantic
import traceback

from .devices import *


type ExpressionType = str | int | float | bool | None


class BaseAction(pydantic.BaseModel):
    action: str
    target: str
    _input: typing.Any = None

    def get_light(self) -> "LightDevice":
        try:
            return connected_lights[self.target]
        except KeyError:
            print(f"action failed: light {self.target} is not connected")
            raise

    def get_sensor(self) -> "SensorDevice":
        try:
            return connected_sensors[self.target]
        except KeyError:
            print(f"action failed: sensor {self.target} is not connected")
            raise
    
    def eval_expr(self, expr: ExpressionType) -> typing.Any:
        return eval(str(expr), {}, {"input": self._input})

    async def run(self, input: typing.Any = None):
        self._input = input


class ActionBlitz(BaseAction):
    action: typing.Literal["blitz"]
    duration: ExpressionType

    async def run(self, input: typing.Any = None):
        await super().run(input)
        print("test blitz action")
        print(f"duration={self.eval_expr(self.duration)}")
        light = self.get_light()
        light.animate_blitz(self.eval_expr(self.duration))


class ActionDummy(BaseAction):
    action: typing.Literal["dummy"]

    async def run(self, input: typing.Any = None):
        await super().run(input)
        print("dummy ran")


type AnyAction = ActionBlitz | ActionDummy


async def run_actions(actions: list[AnyAction], input: typing.Any = None):
    for action in actions:
        try:
            await action.run(input)
        except Exception as e:
            print("action failed:")
            traceback.print_exception(e)
