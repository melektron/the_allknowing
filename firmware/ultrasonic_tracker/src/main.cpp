#include <Arduino.h>

#define PIN_ECHO 26
#define PIN_TRIG 27

void setup()
{
    Serial.begin(115200);
    while (!Serial);


    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT);
}

void loop()
{
}