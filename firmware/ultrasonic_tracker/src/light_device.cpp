/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 16:33

Light Device implementation
*/

#include "light_device.hpp"


LightDevice::LightDevice(std::vector<CRGB> &_buffer)
    : frame_buffer(_buffer)
{}

void LightDevice::setBrightness(uint8_t _br)
{
    // for now just set fastled brightness.
    FastLED.setBrightness(_br);
    // later this will be handled internally to make different stirps different brightness 
    brightness = _br;
    
    FastLED.show();
}

void LightDevice::renderFrame() noexcept
{
    double weight = 0;

    if (background_color != CRGB::Black)
    {
        weight = 1.0f / (running_animations.size() + 1);

        // initialize composition buffer with background color
        for (CRGB &_led : frame_buffer)
            _led.setRGB(
                background_color.r * weight,
                background_color.g * weight,
                background_color.b * weight
            );
        printf("composited bg\n");
    }
    else
    {
        // no background -> don't account for it during compositing to achieve
        // higher brightness levels.
        weight = 1.0f / (running_animations.size() + 1);
        // clear the composite frame buffer
        for (CRGB &_led : frame_buffer)
            _led.setRGB(0, 0, 0);
        printf("ignored bg\n");
    }


    for (auto &animation : running_animations)
    {
        // render frame
        animation->renderFrame();
        // make sure buffer size is compatible
        if (frame_buffer.size() != animation->frame_buffer.size())
        {
            printf("Cannot composite frame of animation because frame buffer size does not match\n");
            continue;
        }
        // composite it onto composition frame buffer
        auto output_it = frame_buffer.begin();
        auto input_it = animation->frame_buffer.begin();
        while (output_it != frame_buffer.end())
        {
            (*output_it) += CRGB(
                input_it->r * weight,
                input_it->g * weight,
                input_it->b * weight
            );
            output_it++;
            input_it++;
        }
        printf("composited one animation\n");
    }

    // remove all the animations that are finished.
    for (auto it = running_animations.begin(); it != running_animations.end(); it++)
    {
        if ((*it)->isComplete())
        {
            it = running_animations.erase(it);
            printf("removed one animation\n");
        }
    }
}

void LightDevice::push() noexcept
{
    // push to LEDs
    FastLED.show();
}

void LightDevice::setBackgroundColor(const CRGB &_color)
{
    background_color = _color;
}

void LightDevice::addAnimation(animation_ptr_t _anim)
{
    running_animations.push_back(_anim);
}

void LightDevice::startBlitzAnimation(const CRGB &_color, int _dur)
{
    addAnimation(
        std::make_shared<anim::BlitzAnimation>(
            frame_buffer.size(),
            _color,
            _dur
        )
    );
}
void LightDevice::startWaveAnimation(
    const double _speed,
    anim::WaveAnimation::direction_t _direction,
    int _starting_position,
    int _width,
    const CRGB &_color
) {
    addAnimation(
        std::make_shared<anim::WaveAnimation>(
            frame_buffer.size(),
            _speed,
            _direction,
            _starting_position,
            _width,
            _color
        )
    );
}
