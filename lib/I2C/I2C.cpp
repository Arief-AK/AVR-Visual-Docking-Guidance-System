#include "I2C.h"

I2C::I2C(uint32_t clock_speed): m_clock_speed(clock_speed){}

void I2C::Begin()
{
    // Set the clock speed
    TWSR = 0;
    TWBR = ((F_CPU / m_clock_speed) - 16) / 2;
    TWCR = (1 << TWEN);
}

void I2C::Start()
{
    TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);
    while (!(TWCR & (1 << TWINT)));
}

void I2C::Stop()
{
    TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
}

void I2C::Write(uint8_t data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t I2C::readAck()
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t I2C::readNack()
{
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

void I2C::writeReg(uint8_t address, uint8_t reg, uint8_t data)
{
    Start();
    Write(address << 1);
    Write(reg);
    Write(data);
    Stop();
}

uint8_t I2C::readReg(uint8_t address, uint8_t reg)
{
    Start();
    Write(address << 1);
    Write(reg);
    Start();
    Write((address << 1) | 1);
    uint8_t data = readNack();
    Stop();
    return data;
}
