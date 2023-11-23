/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 22:09

Animation framework
*/

#pragma once

#include <vector>
#include <FastLED.h>

class LightDevice;

namespace anim
{
    class Animation
    {
    protected:
        friend class ::LightDevice;
        std::vector<CRGB> frame_buffer;

        bool animation_completed = false;
        int frame_counter = 0;

    protected:
        /**
         * @brief function to render the next frame. This is
         * only called when animation is not marked as completed
         * jet. It must not return anything but rather just modify 
         * the frame buffer. This function can set the animation_complete
         * flag to indicate that this was the last frame.
         */
        virtual void renderFrameInternal() noexcept = 0;

    public:
        Animation(int _nr_leds, const CRGB &_default_value = {});
        virtual ~Animation();// = default;

        /**
         * @brief renders the next frame of the animation if it is not
         * already complete.
         */
        void renderFrame() noexcept;

        /**
         * @return true animation is complete and can be deleted, no more frames need to be rendered
         * @return false animation is not complete and needs to be rendered a few more times
         */
        bool isComplete() const noexcept;

    };


    class StaticColorAnimation : public Animation
    {
    protected:
        virtual void renderFrameInternal() noexcept override;
    
    public:
        using Animation::Animation;

        // sets the color of the constant animation
        void setColor(const CRGB &_color) noexcept;
    };


    class BlitzAnimation : public Animation
    {
    protected:
        // number of frames the blitz should last
        const int duration;

        virtual void renderFrameInternal() noexcept override;
    
    public:
        BlitzAnimation(int _nr_leds, const CRGB &_color, int _duration);
    };


    class WaveAnimation : public Animation
    {
    public:
        enum direction_t
        {
            BOTH = 0,
            UP = 1,
            DOWN = 2
        };
    protected:
        // pixels per frame
        const double speed;
        // whether wave should go in both directions or just one
        const direction_t direction;
        // where on the strip the wave should spread out from
        const int starting_position;
        // how wide the wave body should be in pixels. This includes the faded area
        const int width;
        // the wave color
        const CRGB color;

        /**
         * @brief function that creates a smooth curve with the
         * range 0.0 to 1.0 from positions in the range
         * of -width to +width.
         * 
         * @param _position 
         * @return double 
         */
        double waveFunction(double _position) const noexcept;

        /**
         * @brief renders a single wave into the frame buffer
         * at a given position. Returns false
         * if none of the rendered pixels were within
         * the visible frame buffer range anymore.
         * 
         * @param position 
         * @return true was still on screen
         * @return false no longer on screen
         */
        bool renderWave(int position) noexcept;

        virtual void renderFrameInternal() noexcept override;
    
    public:
        WaveAnimation(
            int _nr_leds,
            const double _speed,
            direction_t _direction,
            int _starting_position,
            int _width,
            const CRGB &_color
        );
    };



} // namespace anim
