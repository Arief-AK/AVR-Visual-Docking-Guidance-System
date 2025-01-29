#include <avr/interrupt.h>

#include <MAX7219.h>
#include <Ultrasonic.h>
#include <UART.h>

// Define the SPI pins
#define DATA_PIN   PB3 // MOSI
#define CLK_PIN    PB5 // SCK
#define CS_PIN     PB2 // SS

// Define the Ultrasonic sensors variable
#define STOP_DISTANCE 20
#define SLOW_DISTANCE 30
#define ONWARD_DISTANCE 40

// Ultrasonic 1
#define TRIG_PIN_1 PD2
#define ECHO_PIN_1 PD3
Ultrasonic sensor_1(TRIG_PIN_1, ECHO_PIN_1);

// Ultrasonic 2
#define TRIG_PIN_2 PD4
Ultrasonic sensor_2(TRIG_PIN_2, TRIG_PIN_2, true);


// Define MAX7219 variables
#define NUM_MATRICES 3
#define SCROLL_SPEED 25

// Define UART variables
#define BAUD 9600
#define UBRR F_CPU/16/BAUD-1

// Interrupt variables
volatile long DISTANCE_1 = 0;
volatile long DISTANCE_2 = 0;
volatile bool DISTANCE_MEASURED = false;

ISR(TIMER1_COMPA_vect) {
    // Measure the distance
    sensor_1.MeasureDistance();
    sensor_2.MeasureDistance();

    // Update the distance
    DISTANCE_1 = sensor_1.GetDistance();
    DISTANCE_2 = sensor_2.GetDistance();

    // Set the distance measured flag
    DISTANCE_MEASURED = sensor_1.IsDistanceMeasured() && sensor_2.IsDistanceMeasured();
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

void ScrollText(const char *text, MAX7219* display, UART* uart)
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
        } else if (*p >= 'a' && *p <= 'z') {
            charIndex = *p - 'a' + 26 + 10; // Adjust based on your font array
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

    // Scroll the buffer to the end to ensure the entire message is displayed
    for (uint8_t i = 0; i < NUM_MATRICES * 8; i++) {
        display->_shiftBuffer(buffer, 0x00);
        display->_displayBuffer(buffer);
        _delay_ms(SCROLL_SPEED);
    }
}

int main()
{
    // Initialize the UART
    UART uart(UBRR);

    // Intialise MAX7219 display and Ultrasonic sensor
    MAX7219 display(NUM_MATRICES, DATA_PIN, CLK_PIN, CS_PIN);
    display.SetScrollSpeed(SCROLL_SPEED);

    // Initialise sensor distances
    sensor_1.SetDistances(STOP_DISTANCE, SLOW_DISTANCE, ONWARD_DISTANCE);
    sensor_2.SetDistances(STOP_DISTANCE, SLOW_DISTANCE, ONWARD_DISTANCE);

    // Initialise message variable
    const char* message = "DEFAULT";

    // Initialize Timer1
    Timer1Init();

    // Enable global interrupts
    sei();

    while (true){
        // Display the message if the distance is within the threshold
        if (DISTANCE_MEASURED) {
            // Reset the flag
            DISTANCE_MEASURED = false;

            // Determine the object's position
            if(DISTANCE_1 < DISTANCE_2){
                message = "LEFT";
            } else if(DISTANCE_1 > DISTANCE_2){
                message = "RIGHT";
            } else{
                message = "CENTER";
            }

            // Display the distance and message on UART
            uart.print("Distance1: ");
            uart.print_number(DISTANCE_1);
            uart.print(" cm, Distance2: ");
            uart.print_number(DISTANCE_2);
            uart.println(" cm");

            // Scroll the message on the display
            if(DISTANCE_1 > 0){
                ScrollText(message, &display, &uart);
            }
        }
    }
    return 0;
}