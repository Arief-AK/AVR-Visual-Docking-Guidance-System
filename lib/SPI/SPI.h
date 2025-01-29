#ifndef SPI_H
#define SPI_H

#include <avr/io.h>

class SPI
{
public:
    SPI(uint8_t data_pin, uint8_t clk_pin, uint8_t cs_pin);

    void Send(uint8_t data);

private:
    uint8_t m_data_pin;
    uint8_t m_clk_pin;
    uint8_t m_cs_pin;
};

#endif // SPI_H