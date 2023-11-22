"""
ELEKTRON © 2023 - now
Written by melektron & Nilusink
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

    def get_light(self) -> LightDevice:
        try:
            return connected_lights[self.target]
        except KeyError:
            print(f"action failed: light {self.target} is not connected")
            raise

    def get_sensor(self) -> SensorDevice:
        try:
            return connected_sensors[self.target]
        except KeyError:
            print(f"action failed: sensor {self.target} is not connected")
            raise
    
    def eval_expr(self, expr: ExpressionType) -> typing.Any:
        return eval(str(expr), {}, {"input": self._input})

    async def run(self, input: typing.Any = None) -> None:
        self._input = input
        await self._run_impl();
    
    async def _run_impl(self):
        ...


class ActionSetBrightness(BaseAction):
    action: typing.Literal["brightness"]
    brightness: ExpressionType

    async def _run_impl(self) -> None:
        print(f"brightness={self.eval_expr(self.brightness)}")
        light = self.get_light()
        await light.set_brightness(self.eval_expr(self.brightness))


class ActionSetStaticColor(BaseAction):
    action: typing.Literal["static_color"]
    color: ExpressionType

    async def _run_impl(self) -> None:
        print(f"color={self.eval_expr(self.color)}")
        light = self.get_light()
        await light.set_static_color(self.eval_expr(self.color))


class ActionBlitz(BaseAction):
    action: typing.Literal["blitz"]
    duration: ExpressionType

    async def _run_impl(self, _input: typing.Any = None) -> None:
        print(f"duration={self.eval_expr(self.duration)}")
        light = self.get_light()
        await light.animate_blitz(self.eval_expr(self.duration))


class ActionWave(BaseAction):
    action: typing.Literal["wave"]
    duration: ExpressionType

    async def _run_impl(self, _input: typing.Any = None) -> None:
        print(f"duration={self.eval_expr(self.duration)}")

        light = self.get_light()
        await light.animate_wave(self.eval_expr(self.duration))


class ActionAvoid(BaseAction):
    action: typing.Literal["avoid"]
    speed: ExpressionType

    async def _run_impl(self, _input: typing.Any =  None) -> None:
        print(f"speed={self.eval_expr(self.speed)}")

        light = self.get_light()
        await light.animate_avoid(self.eval_expr(self.speed))


class ActionBlink(BaseAction):
    action: typing.Literal["blink"]
    duration: ExpressionType
    n_blinks: ExpressionType

    async def _run_impl(self, _input: typing.Any = None) -> None:
        print(f"duration={self.eval_expr(self.duration)}, n_blinks={self.eval_expr(self.n_blinks)}")

        light = self.get_light()
        await light.animate_blink(self.eval_expr(self.speed), self.eval_expr(self.n_blinks))


class ActionRainbow(BaseAction):
    action: typing.Literal["rainbow"]
    duration: ExpressionType

    async def _run_impl(self, _input: typing.Any = None) -> None:
        print(f"duration={self.eval_expr(self.duration)}")

        light = self.get_light()
        await light.animate_rainbow(self.eval_expr(self.duration))


class ActionDummy(BaseAction):
    action: typing.Literal["dummy"]

    async def run(self, input: typing.Any = None) -> None:
        await super().run(input)
        print("dummy ran")


type AnyAction = ActionSetBrightness | ActionSetStaticColor | ActionBlitz | ActionDummy


async def run_actions(actions: list[AnyAction], input: typing.Any = None) -> None:
    for action in actions:
        try:
            await action.run(input)
        except Exception as e:
            print("action failed:")
            traceback.print_exception(e)
