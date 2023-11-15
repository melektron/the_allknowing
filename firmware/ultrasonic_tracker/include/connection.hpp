/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
15.11.23, 14:08

implementation of the server connection
*/

#pragma once

#include <ArduinoWebsockets.h>

namespace ws = websockets;

class Connection
{
private:    // state
    ws::WebsocketsClient wsclient;

    // handle for the networking task
    TaskHandle_t networking_task;

    // connected flag
    bool is_connected = false;

private:    // methods

    // websocket library callbacks
    void onMessage(ws::WebsocketsMessage _msg);
    void onEvent(ws::WebsocketsEvent _evt, String _data);   // must use arduino string unfortunately

public:

    Connection();

    /**
     * @brief connects to 
     * 
     * @param _url 
     */
    void connect(const char *_url);

    /**
     * @brief reports a distance to the server
     * 
     * @param _distance the distance of the sensor in cm.
     */
    void report(uint8_t distance);

    /**
     * @brief starts an RTOS task to handle
     * communication.
     * 
     */
    void run();
};