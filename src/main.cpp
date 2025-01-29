#include <avr/interrupt.h>

#include <MAX7219.h>
#include <Ultrasonic.h>
#include <UART.h>

// Define the SPI pins
#define DATA_PIN   PB3 // MOSI
#define CLK_PIN    PB5 // SCK
#define CS_PIN     PB2 // SS

// Define the Ultrasonic sensor variables
#define TRIG_PIN PD2
#define ECHO_PIN PD3
#define STOP_DISTANCE 10
#define SLOW_DISTANCE 15
#define ONWARD_DISTANCE 20
Ultrasonic sensor(TRIG_PIN, ECHO_PIN);

// Define MAX7219 variables
#define NUM_MATRICES 2
#define SCROLL_SPEED 50

// Define UART variables
#define BAUD 9600
#define UBRR F_CPU/16/BAUD-1

// Interrupt variables
volatile long DISTANCE = 0;
volatile bool DISTANCE_MEASURED = false;
const char* MESSAGE = "DEFAULT";

ISR(TIMER1_COMPA_vect) {
    // Measure the distance
    sensor.MeasureDistance();

    // Update the distance
    DISTANCE = sensor.GetDistance();

    // Set the distance measured flag
    DISTANCE_MEASURED = sensor.IsDistanceMeasured();
}

void Timer1Init() {
    // Set up Timer1 with a prescaler of 64 and CTC mode
    TCCR1B |= (1 << WGM12) | (1 << CS11) | (1 << CS10);
    // Set the compare match register for 100ms intervals
    OCR1A = (F_CPU / 64 / 20) - 1;
    // Enable Timer1 compare interrupt
    TIMSK1 |= (1 << OCIE1A);
}

void AddSpace(uint8_t *buffer, MAX7219* display)
{
    for (uint8_t i = 0; i < 3; i++) {
        display->_shiftBuffer(buffer, 0x00);
        display->_displayBuffer(buffer);
        
        // Adjust the delay for scrolling speed
        _delay_ms(SCROLL_SPEED);
    }
}

void ScrollText(const char *text, MAX7219* display)
{
    uint8_t buffer[NUM_MATRICES * 8] = {0}; // Buffer to hold the display data

    // Loop through each character in the text
    for (const char *p = text; *p != '\0'; p++) {
        // Get the character pattern from the font array
        uint8_t charIndex;
        if (*p >= 'A' && *p <= 'Z') {
            charIndex = *p - 'A'; // Adjust based on your font array
        } else if (*p >= '0' && *p <= '9') {
            charIndex = *p - '0' + 26; // Adjust based on your font array
        } else {
            continue; // Skip non-alphanumeric characters
        }

        for (uint8_t col = 0; col < 5; col++) {
            uint8_t columnData = display->_getCharPattern(charIndex, col);
            display->_shiftBuffer(buffer, columnData);
            display->_displayBuffer(buffer);
            
            // Adjust the delay for scrolling speed
            _delay_ms(SCROLL_SPEED);
        }
        AddSpace(buffer, display);
    }
}

int main()
{
    // Initialize the UART
    UART uart(UBRR);

    // Intialise MAX7219 display and Ultrasonic sensor
    MAX7219 display(NUM_MATRICES, DATA_PIN, CLK_PIN, CS_PIN);
    sensor.SetDistances(STOP_DISTANCE, SLOW_DISTANCE, ONWARD_DISTANCE);

    // Initialise message variable
    const char* message = "Onward";

    // Initialize Timer1
    Timer1Init();

    // Enable global interrupts
    sei();

    while (true){
        // Display the message if the distance is within the threshold
        if (DISTANCE_MEASURED) {
            // Reset the flag
            DISTANCE_MEASURED = false;

            // Display the distance and message on UART
            message = sensor.GetDistanceMessage();
            uart.print("Distance: ");
            uart.print_number(DISTANCE);
            uart.println(" cm");
            uart.println(message);

            // Scroll the message on the display
            if(DISTANCE > 0){
                ScrollText(message, &display);
            }
        }
    }
    return 0;
}