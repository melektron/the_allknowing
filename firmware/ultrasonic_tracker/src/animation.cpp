/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 22:19

animation implementation
*/

#include "animation.hpp"

using namespace anim;


Animation::Animation(int _nr_leds, const CRGB &_default_value)
    : frame_buffer(_nr_leds, _default_value)
{}

Animation::~Animation()
{
    printf("destroyed animation instance\n");
}

void Animation::renderFrame() noexcept
{
    if (!animation_completed)
    {
        renderFrameInternal();
        frame_counter++;
    }
}

bool Animation::isComplete() const noexcept
{
    return animation_completed;
}


void StaticColorAnimation::renderFrameInternal() noexcept
{
    // this animation never changes
}

void StaticColorAnimation::setColor(const CRGB &_color) noexcept
{
    for (CRGB &led : frame_buffer)
        led = _color;
}


BlitzAnimation::BlitzAnimation(int _nr_leds, const CRGB &_color, int _duration)
    : Animation(_nr_leds, _color)
    , duration(_duration)
{}

void BlitzAnimation::renderFrameInternal() noexcept
{
    // finish the animation after the duration has been reached.
    if (frame_counter >= duration)
        animation_completed = true;
}
