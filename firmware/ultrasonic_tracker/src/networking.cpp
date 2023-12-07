/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
15.11.23, 14:08

implementation of the server connection
*/

#include <functional>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>

#include "networking.hpp"


namespace pl = std::placeholders;


#define SERVER_URL "ws://" SERVER_IP "/device" 


// helper to print wifi status
static const char *get_wifi_status_string(int status)
{
    switch (status)
    {
    case WL_IDLE_STATUS:
        return "WL_IDLE_STATUS";
    case WL_SCAN_COMPLETED:
        return "WL_SCAN_COMPLETED";
    case WL_NO_SSID_AVAIL:
        return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:
        return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
        return "WL_CONNECTION_LOST";
    case WL_CONNECTED:
        return "WL_CONNECTED";
    case WL_DISCONNECTED:
        return "WL_DISCONNECTED";
    }
    return "N/A";
}


Networking::Networking(std::vector<SensorDevice*> _sensors, std::vector<LightDevice*> _lights)
    : sensors(_sensors)
    , lights(_lights)
{
    // configure IDs
    int subid_count = 0;
    for (auto *sensor : sensors)
        sensor->subdevice_id = subid_count++;
    for (auto *light : lights)
        light->subdevice_id = subid_count++;

    wsclient.onMessage(std::bind(&Networking::onMessage, this, pl::_1));
    wsclient.onEvent(std::bind(&Networking::onEvent, this, pl::_1, pl::_2));
}

void Networking::onMessage(ws::WebsocketsMessage _msg)
{
    printf("Got Message: %s\n", _msg.c_str());
    DynamicJsonDocument jbuffer(1024);
    if (deserializeJson(jbuffer, _msg.c_str()) != DeserializationError::Ok)
    {
        printf("Got invalid json message, ignoring");
    }
    // read message type
    std::string msg_type = jbuffer["type"];
    const int sub = jbuffer["sub"];
    const int sub_sensor = sub;
    const int sub_light = sub - sensors.size();
    if (msg_type == "col")
    {
        uint8_t r = jbuffer["r"];
        uint8_t g = jbuffer["g"];
        uint8_t b = jbuffer["b"];

        lights[sub_light]->setBackgroundColor(CRGB(r, g, b));
    }
    else if (msg_type == "br")
    {
        int br = jbuffer["br"];

        lights[sub_light]->setBrightness((uint8_t)br);
    }
    else if (msg_type == "blitz")
    {
        uint8_t r = jbuffer["r"];
        uint8_t g = jbuffer["g"];
        uint8_t b = jbuffer["b"];
        int dur = jbuffer["dur"];

        lights[sub_light]->startBlitzAnimation(CRGB(r, g, b), dur);
    }
    else if (msg_type == "wave")
    {
        uint8_t r = jbuffer["r"];
        uint8_t g = jbuffer["g"];
        uint8_t b = jbuffer["b"];
        double s = jbuffer["s"];    // speed
        anim::WaveAnimation::direction_t dir = jbuffer["dir"];
        int pos = jbuffer["pos"];
        int w = jbuffer["w"];
        
        lights[sub_light]->startWaveAnimation(s, dir, pos, w, CRGB(r, g, b));
    }
    else if (msg_type == "blink")
    {
        uint8_t r = jbuffer["r"];
        uint8_t g = jbuffer["g"];
        uint8_t b = jbuffer["b"];
        int on = jbuffer["on"];
        int off = jbuffer["off"];
        int n = jbuffer["n"];
        
        lights[sub_light]->startBlinkAnimation(CRGB(r, g, b), on, off, n);
    }

}

void Networking::onEvent(ws::WebsocketsEvent _evt, String _data)
{
    if(_evt == ws::WebsocketsEvent::ConnectionOpened) {
        printf("WS Connnection Opened\n");
        server_connected = true;
        // send identification message
        DynamicJsonDocument jbuffer(1024);
        jbuffer["type"] = "id";
        jbuffer["mac"] = ESP.getEfuseMac();
        jbuffer["sensors"] = sensors.size();
        jbuffer["lights"] = lights.size();
        String outbuffer;
        serializeJson(jbuffer, outbuffer);
        wsclient.send(outbuffer);

    } else if(_evt == ws::WebsocketsEvent::ConnectionClosed) {
        printf("WS Connnection Closed\n");
        server_connected = false;
        disconnect_time = millis();
    } else if(_evt == ws::WebsocketsEvent::GotPing) {
        printf("WS Got a Ping!\n");
    } else if(_evt == ws::WebsocketsEvent::GotPong) {
        printf("WS Got a Pong!\n");
    }
}

void Networking::connectWiFi()
{
    printf("MAC=%llx\n", ESP.getEfuseMac());
    printf("MAC=%s\n", WiFi.macAddress().c_str());
    printf("SSID=%s\n", WIFI_SSID);

    // static IP config
    //IPAddress host_ip(192, 168, 204, 3);
    //IPAddress gateway(192, 168, 207, 254);
    //IPAddress subnet(255, 255, 252, 0);
    //WiFi.config(host_ip, gateway, subnet);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    printf("Connecting...\n");

    // wait for connection
    while (!WiFi.isConnected())
    {
        delay(500);
        printf(".%s\n", get_wifi_status_string(WiFi.status()));
        fflush(stdout);
    }
    printf("\nWiFi connected!\n");
    printf("IP=%s\n", WiFi.localIP().toString().c_str());
}

bool Networking::connectServer()
{
    bool outcome = wsclient.connect(SERVER_URL);
    printf("server connect success: %hhu\n", (uint8_t)outcome);
    if (outcome == false)
    {
        // if connect failed, set status to disconnected (which it should already be anyway)
        // and reset disconnect time so next attempt will be in a few seconds
        server_connected = false;
        disconnect_time = millis();
    }
    return outcome;
}

void Networking::startOTA()
{
            printf("removed one animation\n");
    printf("Configuring OTA...");
    ArduinoOTA.onStart([this]() {
        if (ArduinoOTA.getCommand() == U_FLASH)
            printf("OTA: start firmware update\n");
        else // U_SPIFFS
            printf("OTA: start fs update\n");
        
        this->update_in_progress = true;
        if (this->server_connected)
        {
            this->wsclient.close();
        }
    });
    ArduinoOTA.onEnd([]() {
        printf("OTA: update done\n");
        // don't reset update in progress because esp is restarted after this
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        printf("OTA progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA error: %u (", error);
        if (error == OTA_AUTH_ERROR) printf("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) printf("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) printf("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) printf("Receive Failed");
        else if (error == OTA_END_ERROR) printf("End Failed");
        printf(")\n");
    });
    ArduinoOTA.begin();
}

void Networking::run()
{
    xTaskCreate(
        [](void *_arg){
            uint32_t last_frame_time = millis();
            for (;;)
            {
                Networking *_this = static_cast<Networking*>(_arg);

                // update websocket
                _this->wsclient.poll();
                // update ota
                ArduinoOTA.handle();

                // reconnect WiFi and server if wifi connection lost
                if (!WiFi.isConnected())
                {
                    _this->wsclient.close();
                    WiFi.disconnect(true);

                    _this->connectWiFi();
                }

                // reconnect server
                if (
                    !_this->server_connected &&         // if it is not connected
                    !_this->update_in_progress &&       // and we are not currently updating
                    WiFi.isConnected() &&               // and we have wifi
                    millis() - _this->disconnect_time > 3000    // and server disconnect is more than 3 seconds ago
                ) {
                    _this->connectServer();
                }

                // check for sensor changes
                for (auto *sensor : _this->sensors)
                {
                    if (sensor->hasChanged())
                    {
                        _this->sendSensorReport(*sensor);
                    }
                }

                // render frame if needed
                if (millis() - last_frame_time >= 50)
                {
                    for (auto &light : _this->lights)
                    {
                        light->renderFrame();
                    }
                    LightDevice::push();

                    last_frame_time = millis();
                }

                // wait a bit for next poll
                delay(5);
            }

            vTaskDelete(NULL);
        },
        "wsclient",
        3000,
        this,
        1,
        &networking_task
    );
    wsclient.poll();
}

void Networking::sendSensorReport(SensorDevice &_sensor)
{
    if (server_connected)
    {
        // send identification message
        DynamicJsonDocument jbuffer(1024);
        jbuffer["type"] = "dist";
        jbuffer["sub"] = _sensor.subdevice_id;
        jbuffer["dist"] = _sensor.getValue();
        String outbuffer;
        serializeJson(jbuffer, outbuffer);
        wsclient.send(outbuffer);
    }
}

bool Networking::updateInProgress() const noexcept
{
    return update_in_progress;
}