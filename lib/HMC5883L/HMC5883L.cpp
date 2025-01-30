#include "HMC5883L.h"

HMC5883L::HMC5883L() : m_x(0), m_y(0), m_z(0)
{
    m_i2c.Begin();
}

void HMC5883L::_readX()
{
    m_x_bits[0] = m_i2c.readReg(0x1E, 0x03);
    m_x_bits[1] = m_i2c.readReg(0x1E, 0x04);
}

void HMC5883L::_readZ()
{
    m_z_bits[0] = m_i2c.readReg(0x1E, 0x05);
    m_z_bits[1] = m_i2c.readReg(0x1E, 0x06);
}

void HMC5883L::_readY()
{
    m_y_bits[0] = m_i2c.readReg(0x1E, 0x07);
    m_y_bits[1] = m_i2c.readReg(0x1E, 0x08);
}

void HMC5883L::ReadCompass()
{
    // Read the values
    _readX();
    _readY();
    _readZ();

    // Combine the bits
    m_x = (m_x_bits[0] << 8) | m_x_bits[1];
    m_y = (m_y_bits[0] << 8) | m_y_bits[1];
    m_z = (m_z_bits[0] << 8) | m_z_bits[1];
}

int16_t HMC5883L::GetX() const
{
    return m_x;
}

int16_t HMC5883L::GetY() const
{
    return m_y;
}

int16_t HMC5883L::GetZ() const
{
    return m_z;
}