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

void LightDevice::setFullColor(const CRGB &_color)
{
    for (CRGB &led : frame_buffer)
    {
        led = _color;
    }
    FastLED.show();
}

