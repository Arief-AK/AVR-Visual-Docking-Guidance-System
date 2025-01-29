#include "MAX7219.h"

// Character patterns for a 5x7 font
const uint8_t font5x7[][5] PROGMEM = {
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 'E'
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 'Q'
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // 'R'
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 'V'
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 'Z'
    {0x7C, 0x54, 0x54, 0x54, 0x44}, // '0'
    {0x00, 0x44, 0x7C, 0x40, 0x00}, // '1'
    {0x74, 0x54, 0x54, 0x54, 0x5C}, // '2'
    {0x54, 0x54, 0x54, 0x54, 0x7C}, // '3'
    {0x1C, 0x10, 0x10, 0x10, 0x7C}, // '4'
    {0x5C, 0x54, 0x54, 0x54, 0x74}, // '5'
    {0x7C, 0x54, 0x54, 0x54, 0x74}, // '6'
    {0x04, 0x04, 0x04, 0x04, 0x7C}, // '7'
    {0x7C, 0x54, 0x54, 0x54, 0x7C}, // '8'
    {0x5C, 0x54, 0x54, 0x54, 0x7C}, // '9'
    // Add more characters as needed
};

MAX7219::MAX7219(uint8_t num_matrices, uint8_t data_pin, uint8_t clk_pin, uint8_t cs_pin): m_num_matrices{num_matrices}, m_data_pin{data_pin},
m_clk_pin{clk_pin}, m_cs_pin{cs_pin}, m_spi{SPI(data_pin, clk_pin, cs_pin)}
{
    for (int i = 0; i < m_num_matrices; i++){
        _send(0x0C, 0x01); // Shutdown register: Normal operation
        _send(0x0F, 0x00); // Display test: Off
        _send(0x0B, 0x07); // Scan limit: Display digits 0-7
        _send(0x09, 0x00); // Decode mode: No decode
        _send(0x0A, 0x0F); // Intensity: Maximum brightness
    }
    
}

void MAX7219::SetScrollSpeed(uint8_t speed)
{
    m_scroll_speed = speed;
}

void MAX7219::_send(uint8_t address, uint8_t value)
{
    PORTB &= ~(1 << m_cs_pin);      // Bring CS low
    
    for (int i = 0; i < m_num_matrices; i++) {
        m_spi.Send(address);        // Send the register address
        m_spi.Send(value);          // Send the data
    }
    
    PORTB |= (1 << m_cs_pin);       // Bring CS high
}

uint8_t MAX7219::_getCharPattern(uint8_t charIndex, uint8_t col)
{
    return pgm_read_byte(&font5x7[charIndex][col]);
}

void MAX7219::_shiftBuffer(uint8_t *buffer, uint8_t columnData)
{
    for (uint8_t i = 0; i < m_num_matrices * 8 - 1; i++) {
        buffer[i] = buffer[i + 1];
    }
    buffer[m_num_matrices * 8 - 1] = columnData;
}

void MAX7219::_displayBuffer(uint8_t *buffer)
{
    for (uint8_t row = 1; row <= 8; row++) {
        uint16_t rowData = 0;
        for (uint8_t matrix = 0; matrix < m_num_matrices; matrix++) {
            rowData <<= 8;
            rowData |= buffer[matrix * 8 + row - 1];
        }
        _displayRow(row, rowData);
    }
}

void MAX7219::_displayRow(uint8_t row, uint16_t data)
{
    PORTB &= ~(1 << m_cs_pin);      // Bring CS low
    
    for (int i = 0; i < m_num_matrices; i++) {
        uint8_t matrixData = (data >> (8 * (m_num_matrices - i - 1))) & 0xFF;
        m_spi.Send(row);            // Send the row address
        m_spi.Send(matrixData);     // Send the corresponding matrix data
    }
    
    PORTB |= (1 << m_cs_pin);       // Bring CS high
}

void MAX7219::Clear()
{
    for (uint8_t i = 1; i <= 8; i++) {
        _send(i, 0x00); // Clear each row
    }
}
