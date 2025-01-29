#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

class Ultrasonic
{
public:
    Ultrasonic(uint8_t trigPin, uint8_t echoPin);

    void SetDistances(long stopDistance, long slowDistance, long onwardDistance);

    void MeasureDistance();
    char* GetDistanceMessage();

    long GetDistance() {return m_distance;}
    bool IsDistanceMeasured() {return m_distanceMeasured;}

private:
    uint8_t m_trigPin, m_echoPin;
    uint8_t m_stopDistance, m_slowDistance, m_onwardDistance;
    volatile long m_distance;
    volatile bool m_distanceMeasured;

    unsigned long _micros();
};

#endif // ULTRASONIC_H