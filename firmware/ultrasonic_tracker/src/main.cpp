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


#define USE_ULTRASOUND1
//#define USE_STRIP0
//#define USE_STRIP1
#define USE_STRIP2

#ifdef USE_ULTRASOUND1
#define PIN_ECHO 26
#define PIN_TRIGGER 25
SensorDevice ultrasound1;
#endif

#ifdef USE_STRIP0
#define PIN_LED0 25
std::vector<CRGB> leds0(80);
LightDevice strip0(leds0);
#endif


#ifdef USE_STRIP1
#define PIN_LED1 26
std::vector<CRGB> leds1(79);
LightDevice strip1(leds1);
#endif

#ifdef USE_STRIP2
#define PIN_LED2 27
std::vector<CRGB> leds2(30);
LightDevice strip2(leds2);
#endif


Networking net(
    {
#ifdef USE_ULTRASOUND1
        &ultrasound1,
#endif
    },
    {
#ifdef USE_STRIP0
        &strip0,
#endif
#ifdef USE_STRIP1
        &strip1,
#endif
#ifdef USE_STRIP2
        &strip2,
#endif
    }
);

#ifdef USE_ULTRASOUND1
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

    // first average is calculated here
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

        delay(15);
    }
    
    float avg = 0;
    for (auto sample : samples)
        avg += sample;
    avg /= NR_SAMPLES;

    // report to server
    //printf("distance: %f\n", avg);
    ultrasound1.setValue(avg);
}
#endif


void setup()
{
    Serial.begin(115200);

#ifdef USE_ULTRASOUND1
    // initialize pins for distance sensor
    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);
#endif

#ifdef USE_STRIP0
    FastLED.setBrightness(255);
    strip0.initialize<PIN_LED0>();
#endif
#ifdef USE_STRIP1
    FastLED.setBrightness(255);
    strip1.initialize<PIN_LED1>();
#endif
#ifdef USE_STRIP2
    FastLED.setBrightness(255);
    strip2.initialize<PIN_LED2>();
#endif

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
#ifdef USE_ULTRASOUND1
    processUltrasoundSensor();
#else
    delay(1000);
#endif
}