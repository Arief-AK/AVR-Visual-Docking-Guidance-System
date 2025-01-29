#include "SPI.h"

SPI::SPI(uint8_t data_pin, uint8_t clk_pin, uint8_t cs_pin): m_data_pin{data_pin}, m_clk_pin{clk_pin}, m_cs_pin{cs_pin}
{
    // Set MOSI, SCK, and SS as outputs
    DDRB |= (1 << m_data_pin) | (1 << m_clk_pin) | (1 << m_cs_pin);
    
    // Enable SPI, set as master, and clock rate to fosc/16
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void SPI::Send(uint8_t data)
{
    // Load data into the SPI data register and wait for transmission to complete
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
}
