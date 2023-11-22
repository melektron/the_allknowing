/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 14:38

class representing a Sensor that can be connected to the server
*/

#pragma once

#include <cstdint>


class SensorDevice
{
private:
    int value = 0;
    mutable bool has_changed = false;

    int subdevice_id = 0;
    friend class Networking;

public:
    /**
     * @return true if value has changed
     * @return false if value has not changed
     */
    bool hasChanged() const noexcept;

    /**
     * @brief returns current value of sensor and resets the 
     * has changed flag.
     * 
     * @return int 
     */
    int getValue() const noexcept;

    /**
     * @brief Set a new value and set status to changed.
     * 
     * @param _value new value
     */
    void setValue(int _value) noexcept; 

};