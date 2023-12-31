/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 22:19

animation implementation
*/

#include <cmath>

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


WaveAnimation::WaveAnimation(
    int _nr_leds,
    const double _speed,
    direction_t _direction,
    int _starting_position,
    int _width,
    const CRGB &_color
)
    : Animation(_nr_leds)
    , speed(_speed)
    , direction(_direction)
    , starting_position(_starting_position)
    , width(_width)
    , color(_color)
{}

double WaveAnimation::waveFunction(double _position) const noexcept
{
    /*
    Produces the following Wave function:
                                   __|__
                                ...  |  ...                         
                              ..     |     ..                       
                            ..       |       ..                     
                           .         |         .                    
                         ..          |           .                  
                        .            |            .                 
                       .             |             .                
                     ..              |               .              
                    .                |                .             
                   .                 |                 .            
                 ..                  |                  ..          
                .                    |                    .         
              ..                     |                     ..       
            ..                       |                       ..     
       ...-`                         |                         `-...
    ---|-----------------------------|-----------------------------|---
    -width                           0                          +width
    */
    return (std::sin((_position / width) * M_PI + M_PI_2) + 1) / 2;
}

bool WaveAnimation::renderWave(int position) noexcept
{
    bool on_screen = false;

    for (int i = 0; i < width; i++)
    {
        double intensity = waveFunction(i);
        CRGB pixel_color(
            color.r * intensity,
            color.g * intensity,
            color.b * intensity
        );

        int pos_high = position + i;
        int pos_low = position - i;
        
        // only draw pixels that are in bounds of the frame buffer
        if (pos_high < frame_buffer.size() && pos_high >= 0)
        {
            frame_buffer[pos_high] += pixel_color;  // add for simple compositing which will simply saturate at 255
            on_screen = true;
        }
        if (pos_low < frame_buffer.size() && pos_low >= 0)
        {
            frame_buffer[pos_low] += pixel_color;  // add for simple compositing which will simply saturate at 255
            on_screen = true;
        }
    }

    return on_screen;
}

void WaveAnimation::renderFrameInternal() noexcept
{
    bool on_screen = false;

    // calculate progress based on frames and speed
    int progress = frame_counter * speed;

    // clear frame
    for (CRGB &_led : frame_buffer)
        _led.setRGB(0, 0, 0);

    // if positive wave is enabled
    if (direction == UP || direction == BOTH)
    {
        // set on_screen if wave was on screen, don't clear if it wasn't
        if (renderWave(starting_position + progress))
            on_screen = true;
    }
    // if negative wave is enabled
    if ((direction == DOWN || direction == BOTH))
    {
        // set on_screen if wave was on screen, don't clear if it wasn't
        if (renderWave(starting_position - progress))
            on_screen = true;
    }

    // if no part of the wave was visible anymore, the animation is complete.
    animation_completed = !on_screen;
}


BlinkAnimation::BlinkAnimation(
    int _nr_leds,
    const CRGB &_color,
    int _on_period,
    int _off_period,
    int _n_blinks
)
    : Animation(_nr_leds)
    , color(_color)
    , on_period(_on_period)
    , off_period(_off_period)
    , n_blinks(_n_blinks)
{
    n_blinks_left = n_blinks;

    // if we are initialized with 0 blinks, complete immediately
    if (n_blinks_left <= 0)
        animation_completed = true;
}

void BlinkAnimation::renderFrameInternal() noexcept
{
    // don't do anything if complete (should only happen once if nblinks already starts at 0)
    if (n_blinks_left <= 0)
        return;
    
    // frame counter for current phase
    int phase_frame = frame_counter % (on_period + off_period);

    // when first phase starts, turn light on for the first time (now in on period)
    if (phase_frame == 0 && on_period > 0)
    {
        for (CRGB &_led : frame_buffer)
            _led = color;
    }
    // when exiting the on period, turn off light (now in off period)
    else if (phase_frame == on_period && off_period > 0)
    {
        for (CRGB &_led : frame_buffer)
            _led.setRGB(0, 0, 0);
    }

    // finish the animation after all frames are completed
    if (frame_counter >= (on_period + off_period) * n_blinks - 1)
        animation_completed = true;
}