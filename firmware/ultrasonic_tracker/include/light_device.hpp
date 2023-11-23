/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 15:05

Class representing a light device
*/

#pragma once

#include <FastLED.h>
#include <list>
#include <memory>

#include "animation.hpp"


class LightDevice
{
private:
    int subdevice_id = 0;
    friend class Networking;

    // final frame buffer
    std::vector<CRGB> &frame_buffer;
    // total virtual brightness (not FastLED brightness)
    uint8_t brightness = 0;

    // global background color
    CRGB background_color;

    using animation_ptr_t = std::shared_ptr<anim::Animation>;

    // list of all currently running animations (dynamically allocated
    // because animations can be created on the fly.
    std::list<animation_ptr_t> running_animations;

public:
    LightDevice(std::vector<CRGB> &_buffer);

    template<int _Pin>
    void initialize()
    {
        FastLED.addLeds<WS2812B, _Pin, GRB>(frame_buffer.data(), frame_buffer.size());
    }

    /**
     * @brief renders a single frame of every running animation,
     * composites all frames into one according to a evenly distributed
     * weighted average, and applies global brightness modifier.
     */
    void renderFrame() noexcept;

    /**
     * @brief globally pushes all frame buffers
     * to the LED strips.
     */
    static void push() noexcept;

    /**
     * @brief sets the brightness value of the strip.
     * This will scale all colors to match the brightness.
     * 
     * @param _br 
     */
    void setBrightness(uint8_t _br);
    
    /**
     * @brief set the background color of all LEDs.
     * Animations are composited over this. If it is 0,
     * there will be no background color.
     * 
     * @param _color 
     */
    void setBackgroundColor(const CRGB &_color);

    /**
     * @brief registers a new animation in the running animations. 
     * It will be rendered and processed until it completes.
     * 
     * @param _anim shared pointer to animation instance
     */
    void addAnimation(animation_ptr_t _anim);

    /**
     * @brief creates a blitz animation
     * 
     * @param _color color of blitz
     * @param _dur duration of blitz in frames
     */
    void startBlitzAnimation(const CRGB &_color, int _dur);

    void startWaveAnimation(
        const double _speed,
        anim::WaveAnimation::direction_t _direction,
        int _starting_position,
        int _width,
        const CRGB &_color
    );
};