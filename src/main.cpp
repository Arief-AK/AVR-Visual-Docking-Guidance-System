#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#define DATA_PIN   PB3 // MOSI
#define CLK_PIN    PB5 // SCK
#define CS_PIN     PB2 // SS

// Define the number of cascaded MAX7219 matrices
#define NUM_MATRICES 2

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

uint8_t getCharPattern(uint8_t charIndex, uint8_t col) {
    return pgm_read_byte(&font5x7[charIndex][col]);
}

void shiftBuffer(uint8_t* buffer, uint8_t columnData) {
    for (uint8_t i = 0; i < NUM_MATRICES * 8 - 1; i++) {
        buffer[i] = buffer[i + 1];
    }
    buffer[NUM_MATRICES * 8 - 1] = columnData;
}

void displayBuffer(uint8_t* buffer) {
    for (uint8_t row = 1; row <= 8; row++) {
        uint16_t rowData = 0;
        for (uint8_t matrix = 0; matrix < NUM_MATRICES; matrix++) {
            rowData <<= 8;
            rowData |= buffer[matrix * 8 + row - 1];
        }
        MAX7219_displayRow(row, rowData);
    }
}

void addSpace(uint8_t* buffer) {
    for (uint8_t i = 0; i < 3; i++) {
        shiftBuffer(buffer, 0x00);
        displayBuffer(buffer);
        _delay_ms(100); // Adjust the delay for scrolling speed
    }
}

void scrollText(const char* text) {
    uint8_t buffer[NUM_MATRICES * 8] = {0}; // Buffer to hold the display data

    // Loop through each character in the text
    for (const char* p = text; *p != '\0'; p++) {
        // Get the character pattern from the font array
        uint8_t charIndex;
        if (*p >= 'A' && *p <= 'Z') {
            charIndex = *p - 'A'; // Adjust based on your font array
        } else if (*p >= '0' && *p <= '9') {
            charIndex = *p - '0' + 26; // Adjust based on your font array
        } else {
            continue; // Skip characters not in the font array
        }

        for (uint8_t col = 0; col < 5; col++) {
            uint8_t columnData = getCharPattern(charIndex, col);
            shiftBuffer(buffer, columnData);
            displayBuffer(buffer);
            _delay_ms(100); // Adjust the delay for scrolling speed
        }

        addSpace(buffer);
    }
}

int main() {
    SPI_init();
    MAX7219_init();
    MAX7219_clear();

    const char* message = "HELLO WORLD 0123456789"; // Your scrolling text message
    while (1) {
        scrollText(message);
    }

    return 0;
}