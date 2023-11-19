/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
15.11.23, 14:08

implementation of the server connection
*/

#include <functional>
#include <ArduinoOTA.h>

#include "networking.hpp"


namespace pl = std::placeholders;


#define SERVER_URL "ws://" SERVER_IP "/sensor" 


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


Networking::Networking()
{
    wsclient.onMessage(std::bind(&Networking::onMessage, this, pl::_1));
    wsclient.onEvent(std::bind(&Networking::onEvent, this, pl::_1, pl::_2));
}

void Networking::onMessage(ws::WebsocketsMessage _msg)
{
    printf("Got Message: %s\n", _msg.c_str());
}

void Networking::onEvent(ws::WebsocketsEvent _evt, String _data)
{
    if(_evt == ws::WebsocketsEvent::ConnectionOpened) {
        printf("WS Connnection Opened\n");
        server_connected = true;
        // send MAC address
        char buffer[101] = {0};
        snprintf(buffer, 100, "{\"type\":\"id\",\"mac\":%llu,\"subcount\":1}", ESP.getEfuseMac());
        wsclient.send(buffer);

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
                
                delay(10);
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

void Networking::report(uint8_t _distance)
{
    if (server_connected)
    {
        char buffer[101] = {0};
        snprintf(buffer, 100, "{\"type\":\"dist\",\"sub\":0,\"dist\":%hhu}", _distance);
        wsclient.send(buffer);
    }
}

bool Networking::updateInProgress() const noexcept
{
    return update_in_progress;
}