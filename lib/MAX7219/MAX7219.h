#ifndef MAX7219_H
#define MAX7219_H

#include <avr/pgmspace.h>

#include <SPI.h>

class MAX7219
{
public:
    MAX7219(uint8_t num_matrices, uint8_t data_pin, uint8_t clk_pin, uint8_t cs_pin);

    void SetScrollSpeed(uint8_t speed);
    void Clear();

    uint8_t _getCharPattern(uint8_t charIndex, uint8_t col);
    void _shiftBuffer(uint8_t* buffer, uint8_t columnData);
    void _displayBuffer(uint8_t* buffer);

private:
    uint8_t m_num_matrices, m_data_pin, m_clk_pin, m_cs_pin, m_scroll_speed;
    SPI m_spi;

    void _send(uint8_t address, uint8_t value);
    void _displayRow(uint8_t row, uint32_t data);
};

#endif  // MAX7219_H