#include "Ultrasonic.h"

Ultrasonic::Ultrasonic(uint8_t trigPin, uint8_t echoPin): m_trigPin(trigPin), m_echoPin(echoPin),
m_distance(0), m_distanceMeasured(false)
{
    // Set the trigger pin as an output
    DDRD |= (1 << m_trigPin);
    // Set the echo pin as an input
    DDRD &= ~(1 << m_echoPin);
    
    // // Set the echo pin to trigger on a rising edge
    // EICRA |= (1 << ISC00);
    // // Enable the external interrupt
    // EIMSK |= (1 << INT0);
}

void Ultrasonic::SetDistances(long stopDistance, long slowDistance, long onwardDistance)
{
    m_stopDistance = stopDistance;
    m_slowDistance = slowDistance;
    m_onwardDistance = onwardDistance;
}

unsigned long Ultrasonic::_micros()
{
    return (unsigned long)(TCNT1 * (64.0 / F_CPU) * 1000000.0);
}

void Ultrasonic::MeasureDistance()
{
    // Send a 10us pulse to trigger the sensor
    PORTD &= ~(1 << m_trigPin);
    _delay_us(2);
    PORTD |= (1 << m_trigPin);
    _delay_us(10);
    PORTD &= ~(1 << m_trigPin);

    // Wait for the echo to be received
    while (!(PIND & (1 << m_echoPin)));
    long startTime = _micros();
    while (PIND & (1 << m_echoPin));
    long travelTime = _micros() - startTime;

    // Calculate the distance in cm
    m_distance = travelTime / 58;
    m_distanceMeasured = true;
}

char *Ultrasonic::GetDistanceMessage()
{
    static char message[7];

    // Determine the distance message
    if (m_distance <= m_stopDistance - 5)
    {
        strcpy(message, "DANGER");
    } else if (m_distance <= m_stopDistance)
    {
        strcpy(message, "STOP");
    } else if (m_distance <= m_slowDistance && m_distance > m_stopDistance)
    {
        strcpy(message, "SLOW");
    } else if (m_distance >= m_onwardDistance)
    {
        itoa(m_distance, message, 10);
    }

    return message;
}
