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



} // namespace anim
