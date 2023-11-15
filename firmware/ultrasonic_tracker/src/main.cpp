/*
ELEKTRON © 2023 - now
Written by melektron & MrFlunter
www.elektron.work
08.11.23, 16:30
*/

#include <Arduino.h>
#include <WiFi.h>

#include "connection.hpp"


#define PIN_ECHO 26
#define PIN_TRIGGER 25

#define SETUP_HTL
#include "secrets.h"
#define SERVER_URL "ws://" SERVER_IP "/sensor" 

Connection con;

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);

    WiFi.begin(WIFI_SSID, WIFI_PSK);

    printf("Connecting.");

    while (!WiFi.isConnected())
    {
        delay(1000);
        printf(".");
    }

    printf("\nWiFi connected!\n");

    con.connect(SERVER_URL);

    con.run();
}

float distance; 

void loop()
{
    digitalWrite(PIN_TRIGGER, HIGH); 
    delayMicroseconds(10); 
    digitalWrite(PIN_TRIGGER, LOW); 

    float duration = pulseIn(PIN_ECHO, HIGH);

    distance = duration * 0.0344 / 2;

    printf("distance: %f\n", distance);
    con.report(distance);
    
    delay(1000);
}