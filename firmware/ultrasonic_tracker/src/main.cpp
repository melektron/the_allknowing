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

#define SETUP_HOME
#include "secrets.h"
#define SERVER_URL "ws://" SERVER_IP "/sensor" 

Connection con;

void setup()
{
    Serial.begin(115200);

    // initialize pins for distance sensor
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);

    printf("MAC=%llx\n", ESP.getEfuseMac());
    printf("MAC=%s\n", WiFi.macAddress().c_str());
    printf("SSID=%s\n", WIFI_SSID);

    // connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    printf("Connecting.");

    // wait for connection
    while (!WiFi.isConnected())
    {
        delay(500);
        printf(".");
        fflush(stdout);
    }
    printf("\nWiFi connected!\n");

    // connect to server
    con.connect(SERVER_URL);

    // start networking task
    con.run();
}

void loop()
{
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
    delay(1000);
}