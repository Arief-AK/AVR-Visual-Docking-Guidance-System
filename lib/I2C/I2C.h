#ifndef I2C_H
#define I2C_H

#include <avr/io.h>

class I2C
{
public:
    I2C(uint32_t clock_speed);

    void Begin();
    void Start();
    void Stop();

    void Write(uint8_t data);

    uint8_t readAck();
    uint8_t readNack();

    void writeReg(uint8_t address, uint8_t reg, uint8_t data);
    uint8_t readReg(uint8_t address, uint8_t reg);

private:
    uint32_t m_clock_speed;
};

#endif // I2C_H