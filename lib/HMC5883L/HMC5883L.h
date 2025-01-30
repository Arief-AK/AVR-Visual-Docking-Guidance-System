#ifndef HMC5883L_H
#define HMC5883L_H

#include <avr/io.h>
#include <I2C.h>

class HMC5883L
{
public:
    HMC5883L();

    void ReadCompass();

    int16_t GetX() const;
    int16_t GetY() const;
    int16_t GetZ() const;

private:
    I2C m_i2c = I2C(100000);

    uint8_t m_x_bits[2];
    uint8_t m_y_bits[2];
    uint8_t m_z_bits[2];

    int16_t m_x, m_y, m_z;

    void _readX();
    void _readZ();
    void _readY();
};


#endif // HMC5883L_H