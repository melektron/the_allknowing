#include <Arduino.h>

#define PIN_ECHO 17
#define PIN_TRIG 16

void setup()
{
    Serial.begin(115200);
    while (!Serial);


    pinMode(PIN_TRIG, OUTPUT);
    pinMode(PIN_ECHO, INPUT_PULLUP);
}

double last_distance = 0;

void loop()
{
    digitalWrite(PIN_TRIG, HIGH); 
    delayMicroseconds(10); 
    //delay(500);
    digitalWrite(PIN_TRIG, LOW); 

    int duration = pulseIn(PIN_ECHO, HIGH, 8000);

    printf("duration=%d\n", duration);

    last_distance = duration * 0.0344 / 2;
    //last_distance = (last_distance / max_distance) * 100;

    printf("distance: %lf\n", last_distance);
    
    delay(300);
}