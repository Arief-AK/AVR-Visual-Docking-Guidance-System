#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>

// Define the SPI pins
#define DATA_PIN   PB3 // MOSI
#define CLK_PIN    PB5 // SCK
#define CS_PIN     PB2 // SS

// Define the Ultrasonic sensor pins
#define TRIG_PIN PD2
#define ECHO_PIN PD3

// Define the number of cascaded MAX7219 matrices
#define NUM_MATRICES 2

// Define the built-in LED pin
#define LED_PIN PB5

// Define the baud rate for UART
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Define Scroll Speed
#define SCROLL_SPEED 50

// Define Distance Variables
#define STOP_DISTANCE 10
#define SLOW_DISTANCE 15
#define ONWARD_DISTANCE 20

volatile long distance = 0;
volatile bool distanceMeasured = false;

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

void ultrasonic_init() {
    DDRD |= (1 << TRIG_PIN);  // Set TRIG_PIN as output
    DDRD &= ~(1 << ECHO_PIN); // Set ECHO_PIN as input
}

unsigned long micros() {
    return (unsigned long)(TCNT1 * (64.0 / F_CPU) * 1000000.0);
}

ISR(TIMER1_COMPA_vect) {
    // Send a 10us pulse to trigger the sensor
    PORTD &= ~(1 << TRIG_PIN);
    _delay_us(2);
    PORTD |= (1 << TRIG_PIN);
    _delay_us(10);
    PORTD &= ~(1 << TRIG_PIN);

    // Wait for the echo to be received
    while (!(PIND & (1 << ECHO_PIN)));
    long startTime = micros();
    while (PIND & (1 << ECHO_PIN));
    long travelTime = micros() - startTime;

    // Calculate the distance in cm
    distance = travelTime / 58;
    distanceMeasured = true;
}

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
    for (int i = 0; i < NUM_MATRICES; i++) {
        MAX7219_send(0x0C, 0x01); // Shutdown register: Normal operation
        MAX7219_send(0x0F, 0x00); // Display test: Off
        MAX7219_send(0x0B, 0x07); // Scan limit: Display digits 0-7
        MAX7219_send(0x09, 0x00); // Decode mode: No decode
        MAX7219_send(0x0A, 0x0F); // Intensity: Maximum brightness
    }
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
        _delay_ms(SCROLL_SPEED); // Adjust the delay for scrolling speed
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
            _delay_ms(SCROLL_SPEED); // Adjust the delay for scrolling speed
        }

        addSpace(buffer);
    }
}

void timer1_init() {
    // Set up Timer1 with a prescaler of 64 and CTC mode
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
    // Set the compare match register for 100ms intervals
    OCR1A = (F_CPU / 64 / 20) - 1;
    // Enable Timer1 compare interrupt
    TIMSK1 |= (1 << OCIE1A);
}

void uart_init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    // Enable transmitter
    UCSR0B = (1 << TXEN0);
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void uart_transmit(unsigned char data) {
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    // Put data into buffer, sends the data
    UDR0 = data;
}

void uart_print(const char* str) {
    while (*str) {
        uart_transmit(*str++);
    }
}

void uart_println(const char* str) {
    uart_print(str);
    uart_transmit('\r');
    uart_transmit('\n');
}

void uart_print_number(long num) {
    char buffer[10];
    itoa(num, buffer, 10);
    uart_print(buffer);
}

int main() {
    SPI_init();
    MAX7219_init();
    MAX7219_clear();
    ultrasonic_init();
    timer1_init();
    uart_init(MYUBRR);

    // Initialize the built-in LED pin as output
    DDRB |= (1 << LED_PIN);

    const char* message = "Onward"; // Your scrolling text message

    // Enable global interrupts
    sei();

    while (1) {
        if (distanceMeasured) {
            distanceMeasured = false;
            uart_print("Distance: ");
            uart_print_number(distance);
            uart_println(" cm");

            // Determine the message based on the distance
            if(distance > 0){
              if(distance <= STOP_DISTANCE) {
                  uart_println("STOP");
                  message = "STOP";
              } else if(distance <= SLOW_DISTANCE && distance > STOP_DISTANCE) {
                  uart_println("SLOW");
                  message = "SLOW";
              } else if(distance >= SLOW_DISTANCE){
                MAX7219_clear();
              }
            }

            // Display the message if the distance is within the threshold
            if(distance > 0) {
              scrollText(message);
        }
      }
    }
    return 0;
}