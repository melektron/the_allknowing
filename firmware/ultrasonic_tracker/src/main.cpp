/*
ELEKTRON © 2023 - now
Written by melektron & MrFlunter
www.elektron.work
08.11.23, 16:30
*/

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

#define PIN_ECHO 17
#define PIN_TRIGGER 16

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_TRIGGER, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);

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
    
    delay(300);
}