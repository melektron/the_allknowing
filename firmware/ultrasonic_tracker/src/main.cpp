/*
ELEKTRON © 2023 - now
Written by melektron & MrFlunter
www.elektron.work
08.11.23, 16:30
*/

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "networking.hpp"


#define PIN_ECHO 26
#define PIN_TRIGGER 25


Networking con;


void setup()
{
    Serial.begin(115200);

    // initialize pins for distance sensor
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);

    // connect to wifi
    con.connectWiFi();

    // start OTA service
    con.startOTA();

    // connect to server
    con.connectServer();

    // start networking task
    con.run();
}

void loop()
{
    // if update in progress, stop doing anything in the meantime
    if (con.updateInProgress())
    {
        delay(1000);
        return;
    }
    
    // send ultrasonic pulse
    digitalWrite(PIN_TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(PIN_TRIGGER, LOW);

    // wait for reflection
    float duration = pulseIn(PIN_ECHO, HIGH);
    // calculate distance
    float distance = duration * 0.0344 / 2;

    // report to server
    printf("distance: %f\n", distance);
    con.report(distance);

    // wait a bit
    delay(50);
}