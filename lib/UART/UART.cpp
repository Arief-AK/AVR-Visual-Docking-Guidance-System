#include "UART.h"

UART::UART(unsigned int ubrr) :m_ubrr{ubrr}
{
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;
    
    // Enable transmitter
    UCSR0B = (1 << TXEN0);
    
    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART::_transmit(unsigned char data)
{
    // Wait for empty transmit buffer
    while (!(UCSR0A & (1 << UDRE0)));
    
    // Put data into buffer, sends the data
    UDR0 = data;
}

void UART::print(const char* str)
{
    while (*str) {
        _transmit(*str++);
    }
}

void UART::println(const char* str)
{
    print(str);
    _transmit('\r');
    _transmit('\n');
}

void UART::print_number(int number)
{
    char buffer[10];
    itoa(number, buffer, 10);
    print(buffer);
}