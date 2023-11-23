/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 14:58
*/

#include "sensor_device.hpp"


bool SensorDevice::hasChanged() const noexcept
{
    return has_changed;
}

int SensorDevice::getValue() const noexcept
{
    has_changed = false;
    return value;
}

void SensorDevice::setValue(int _value) noexcept
{
    has_changed = true;
    value = _value;
}