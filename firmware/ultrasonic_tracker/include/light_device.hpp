/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 15:05

Class representing a light device
*/

#pragma once

#include <FastLED.h>
#include <vector>


class LightDevice
{
private:
    int subdevice_id = 0;
    friend class Networking;

    // frame buffer
    std::vector<CRGB> &frame_buffer;

    uint8_t brightness = 0;

public:
    LightDevice(std::vector<CRGB> &_buffer);

    template<int _Pin>
    void initialize()
    {
        FastLED.addLeds<WS2812B, _Pin, GRB>(frame_buffer.data(), frame_buffer.size());
    }

    /**
     * @brief sets the brightness value of the strip.
     * This will scale all colors to match the brightness.
     * 
     * @param _br 
     */
    void setBrightness(uint8_t _br);
    
    /**
     * @brief sets all LEDs to color 
     * 
     * @param _color 
     */
    void setFullColor(const CRGB &_color);
};