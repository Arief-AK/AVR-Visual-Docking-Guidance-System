#include <avr/io.h>
#include <util/delay.h>

#define DATA_PIN   PB3 // MOSI
#define CLK_PIN    PB5 // SCK
#define CS_PIN     PB2 // SS

// Define the number of cascaded MAX7219 matrices
#define NUM_MATRICES 2

void SPI_init() {
    // Set MOSI, SCK, and SS as outputs
    DDRB |= (1 << DATA_PIN) | (1 << CLK_PIN) | (1 << CS_PIN);
    
    // Enable SPI, set as master, and clock rate to fosc/16
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0);
}

void SPI_send(uint8_t data) {
    SPDR = data;                  // Load data into the SPI data register
    while (!(SPSR & (1 << SPIF))); // Wait for transmission to complete
}

void MAX7219_send(uint8_t address, uint8_t value) {
    PORTB &= ~(1 << CS_PIN);      // Bring CS low
    
    for (int i = 0; i < NUM_MATRICES; i++) {
        SPI_send(address);        // Send the register address
        SPI_send(value);          // Send the data
    }
    
    PORTB |= (1 << CS_PIN);       // Bring CS high
}

void MAX7219_init() {
    MAX7219_send(0x0C, 0x01); // Shutdown register: Normal operation
    MAX7219_send(0x0F, 0x00); // Display test: Off
    MAX7219_send(0x0B, 0x07); // Scan limit: Display digits 0-7
    MAX7219_send(0x09, 0x00); // Decode mode: No decode
    MAX7219_send(0x0A, 0x0F); // Intensity: Maximum brightness
}

void MAX7219_clear() {
    for (uint8_t i = 1; i <= 8; i++) {
        MAX7219_send(i, 0x00); // Clear each row
    }
}

void MAX7219_displayRow(uint8_t row, uint16_t data) {
    PORTB &= ~(1 << CS_PIN);      // Bring CS low

    for (int i = 0; i < NUM_MATRICES; i++) {
        uint8_t matrixData = (data >> (8 * (NUM_MATRICES - i - 1))) & 0xFF;
        SPI_send(row);            // Send the row address
        SPI_send(matrixData);     // Send the corresponding matrix data
    }

    PORTB |= (1 << CS_PIN);       // Bring CS high
}

int main() {
    SPI_init();
    MAX7219_init();
    MAX7219_clear();

    // Example animation: Scroll a pattern across the matrices
    uint16_t pattern = 0b1111111100000000; // Example pattern for 2 matrices

    while (1) {
        for (uint8_t row = 1; row <= 8; row++) {
            MAX7219_displayRow(row, pattern); // Display each row
        }
        _delay_ms(500);

        // Shift the pattern left (scrolling effect)
        pattern = (pattern << 1) | ((pattern >> 15) & 0x01);
    }

    return 0;
}