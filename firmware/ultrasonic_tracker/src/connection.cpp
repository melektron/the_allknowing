/*
ELEKTRON © 2023 - now
Written by melektron
www.elektron.work
15.11.23, 14:08

implementation of the server connection
*/

#include <functional>

#include "connection.hpp"

namespace pl = std::placeholders;

Connection::Connection()
{
    wsclient.onMessage(std::bind(&Connection::onMessage, this, pl::_1));
    wsclient.onEvent(std::bind(&Connection::onEvent, this, pl::_1, pl::_2));
}

void Connection::onMessage(ws::WebsocketsMessage _msg)
{
    printf("Got Message: %s\n", _msg.c_str());
}

void Connection::onEvent(ws::WebsocketsEvent _evt, String _data)
{
    if(_evt == ws::WebsocketsEvent::ConnectionOpened) {
        printf("WS Connnection Opened\n");
        is_connected = true;
        // send MAC address
        char buffer[101] = {0};
        snprintf(buffer, 100, "{\"type\":\"mac\", \"mac\":%llu}", ESP.getEfuseMac());
        wsclient.send(buffer);

    } else if(_evt == ws::WebsocketsEvent::ConnectionClosed) {
        printf("WS Connnection Closed\n");
        is_connected = false;
    } else if(_evt == ws::WebsocketsEvent::GotPing) {
        printf("WS Got a Ping!\n");
    } else if(_evt == ws::WebsocketsEvent::GotPong) {
        printf("WS Got a Pong!\n");
    }
}

void Connection::connect(const char *_url)
{
    wsclient.connect(_url);
}

void Connection::report(uint8_t _distance)
{
    if (is_connected)
    {
        char buffer[101] = {0};
        snprintf(buffer, 100, "{\"type\":\"dist\", \"dist\":%hhu}", _distance);
        wsclient.send(buffer);
    }
}

void Connection::run()
{
    xTaskCreate(
        [](void *_arg){
            for (;;)
            {
                Connection *_this = static_cast<Connection*>(_arg);
                _this->wsclient.poll();
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