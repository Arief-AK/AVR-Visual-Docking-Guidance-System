#ifndef UART_H
#define UART_H

#include <avr/io.h>
#include <stdlib.h>

class UART
{
public:
    UART(unsigned int ubrr);

    void print(const char* str);
    void println(const char* str);
    void print_number(int number);

private:
    unsigned int m_ubrr;

    void _transmit(unsigned char data);
};

#endif // UART_H