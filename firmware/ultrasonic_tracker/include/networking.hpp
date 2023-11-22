/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
15.11.23, 14:08

implementation of the server connection
*/

#pragma once

#include <ArduinoWebsockets.h>
#include <vector>

#include "secrets.h"
#include "sensor_device.hpp"
#include "light_device.hpp"

namespace ws = websockets;

class Networking
{
private:    // state
    ws::WebsocketsClient wsclient;

    // handle for the networking task
    TaskHandle_t networking_task;

    // flag to know whether the server is connected
    bool server_connected = false;
    // flag set while an update is in progress so server will not be reconnected
    bool update_in_progress = false;

    // time used to keep track when server connection was lost
    uint64_t disconnect_time = 0;

    // list of sensors and lights
    std::vector<SensorDevice*> sensors;
    std::vector<LightDevice*> lights;
    

private:    // methods

    // websocket library callbacks
    void onMessage(ws::WebsocketsMessage _msg);
    void onEvent(ws::WebsocketsEvent _evt, String _data);   // must use arduino string unfortunately


    /**
     * @brief reports a distance to the server
     * 
     * @param _sensor the sensor to report
     */
    void sendSensorReport(SensorDevice &_sensor);

public:

    Networking(std::vector<SensorDevice*> _sensors = {}, std::vector<LightDevice*> _lights = {});

    /**
     * @brief initiates the configured WiFi connection.
     * 
     */
    void connectWiFi();

    /**
     * @brief connects to the websocket server
     * @returns whether connection was successful
     */
    bool connectServer();

    /**
     * @brief configures and starts the OTA update service
     * 
     */
    void startOTA();

    /**
     * @brief starts an RTOS task to handle
     * network communication.
     * 
     */
    void run();

    /**
     * @retval true update in progress
     * @retval false no update in progres
     */
    bool updateInProgress() const noexcept;

};