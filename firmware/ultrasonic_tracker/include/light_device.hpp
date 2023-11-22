/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
22.11.23, 15:05

Class representing a light device
*/

#pragma once

class LightDevice
{
private:

    int subdevice_id = 0;
    friend class Networking;
};