/*
ELEKTRON © 2023 - now
Written by melektron & MrFlunter
www.elektron.work
08.11.23, 16:30
*/

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <array>
#include <FastLED.h>

#include "networking.hpp"


#define PIN_ECHO 26
#define PIN_TRIGGER 25
SensorDevice ultrasound1;

#define PIN_LED 27
std::vector<CRGB> leds1(30);
LightDevice strip1(leds1);


Networking net(
    {
        &ultrasound1,
    },
    {
        &strip1
    }
);


void processUltrasoundSensor()
{
    // if update in progress, stop doing anything in the meantime
    if (net.updateInProgress())
    {
        delay(1000);
        return;
    }

#define NR_SAMPLES 3
    static std::array<float, NR_SAMPLES> samples;

    for (int i = 0; i < 3; i++)
    {
        // send ultrasonic pulse
        digitalWrite(PIN_TRIGGER, HIGH);
        delayMicroseconds(10);
        digitalWrite(PIN_TRIGGER, LOW);

        // wait for reflection
        float duration = pulseIn(PIN_ECHO, HIGH);
        // calculate distance
        float distance = duration * 0.0344 / 2;

        samples[i] = distance;

        delay(50);
    }
    
    float avg = 0;
    for (auto sample : samples)
        avg += sample;
    avg /= NR_SAMPLES;

    // report to server
    printf("distance: %f\n", avg);
    ultrasound1.setValue(avg);
}

void setup()
{
    Serial.begin(115200);

    // initialize pins for distance sensor
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);

    FastLED.setBrightness(50);
    strip1.initialize<PIN_LED>();

    // connect to wifi
    net.connectWiFi();

    // start OTA service
    net.startOTA();

    // connect to server
    net.connectServer();

    // start networking task
    net.run();
}

void loop()
{
    processUltrasoundSensor();
}