#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

// ARDUINO SPI Definitions
#define MISO PORTB4       // D12
#define MOSI PORTB3       // D11
#define SCK PORTB5        // D13

// CLOCK SELECTS
#define CS1 PORTB2        // D10
#define CS2 PORTB1        // D9

// Number of MAX7219 matrices in the chain
#define NUM_MATRICES 2

// Character bitmaps for 8x8 matrix (A example)
const uint8_t font[36][8] = {
    // A-Z (Uppercase letters)
    {0b00111100, 0b01000010, 0b01000010, 0b01111110, 0b01000010, 0b01000010, 0b01000010, 0b00000000}, // A
    {0b01111100, 0b01000010, 0b01111100, 0b01000010, 0b01000010, 0b01111100, 0b00000000, 0b00000000}, // B
    {0b00111110, 0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b00111110, 0b00000000, 0b00000000}, // C
    {0b01111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01111100, 0b00000000, 0b00000000}, // D
    {0b01111110, 0b01000000, 0b01111100, 0b01000000, 0b01000000, 0b01111110, 0b00000000, 0b00000000}, // E
    {0b01111110, 0b01000000, 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b00000000, 0b00000000}, // F
    {0b00111110, 0b01000000, 0b01000000, 0b01000110, 0b01000010, 0b00111110, 0b00000000, 0b00000000}, // G
    {0b01000010, 0b01000010, 0b01111110, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // H
    {0b01111110, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b01111110, 0b00000000, 0b00000000}, // I
    {0b00000010, 0b00000010, 0b00000010, 0b00000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // J
    {0b01000010, 0b01000100, 0b01001000, 0b01010000, 0b01100000, 0b01000010, 0b00000000, 0b00000000}, // K
    {0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01111100, 0b00000000, 0b00000000, 0b00000000}, // L
    {0b01000010, 0b01100110, 0b01011010, 0b01000010, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // M
    {0b01000010, 0b01100010, 0b01010010, 0b01001010, 0b01000110, 0b01000010, 0b00000000, 0b00000000}, // N
    {0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // O
    {0b01111100, 0b01000010, 0b01111100, 0b01000000, 0b01000000, 0b01000000, 0b00000000, 0b00000000}, // P
    {0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01001010, 0b00111101, 0b00000000, 0b00000000}, // Q
    {0b01111100, 0b01000010, 0b01111100, 0b01000100, 0b01000010, 0b01000010, 0b00000000, 0b00000000}, // R
    {0b00111110, 0b01000000, 0b00111100, 0b00000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // S
    {0b01111110, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000}, // T
    {0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // U
    {0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00100100, 0b00011000, 0b00000000, 0b00000000}, // V
    {0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b01011010, 0b01100110, 0b00000000, 0b00000000}, // W
    {0b01000010, 0b01000010, 0b00100100, 0b00011000, 0b00100100, 0b01000010, 0b00000000, 0b00000000}, // X
    {0b01000010, 0b01000010, 0b00100100, 0b00011000, 0b00011000, 0b00011000, 0b00000000, 0b00000000}, // Y
    {0b01111110, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b01111110, 0b00000000, 0b00000000}, // Z
    
    // 0-9 (Digits)
    {0b00111100, 0b01000010, 0b01000010, 0b01000010, 0b01000010, 0b00111100, 0b00000000, 0b00000000}, // 0
    {0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00111100, 0b00000000, 0b00000000}, // 1
    {0b00111100, 0b01000010, 0b00000010, 0b00011100, 0b01000000, 0b01111110, 0b00000000, 0b00000000}, // 2
    {0b00111100, 0b01000010, 0b00000010, 0b00011100, 0b00000010, 0b01000010, 0b00111100, 0b00000000}, // 3
    {0b00011100, 0b00101100, 0b01000100, 0b01111110, 0b00000100, 0b00000100, 0b00000100, 0b00000000}, // 4
    {0b01111110, 0b01000000, 0b01000000, 0b00111100, 0b00000010, 0b01000010, 0b00111100, 0b00000000}, // 5
    {0b00111100, 0b01000000, 0b01000000, 0b01111100, 0b01000010, 0b01000010, 0b00111100, 0b00000000}, // 6
    {0b01111110, 0b00000010, 0b00000010, 0b00000100, 0b00001000, 0b00001000, 0b00001000, 0b00000000}, // 7
    {0b00111100, 0b01000010, 0b01000010, 0b00111100, 0b01000010, 0b01000010, 0b00111100, 0b00000000}, // 8
    {0b00111100, 0b01000010, 0b01000010, 0b00111110, 0b00000010, 0b01000000, 0b00111100, 0b00000000}  // 9
};

// Initialize SPI
void SPIInit(){
  DDRB |= (1 << PB2) | (1 << PB3) | (1 << PB5); // Set CS, MOSI, SCK as output
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); // Enable SPI, Master mode, Fosc/16
  PORTB |= (1 << PB2);  // Set CS high
}

// SPI send function
void SPISend(uint8_t data){
  SPDR = data;                   // Load data into SPI Data Register
  while (!(SPSR & (1 << SPIF))); // Wait until transmission complete
}

// Send data to MAX7219
void MAX7219_Send(uint8_t cs_pin, uint8_t reg, uint8_t data) {
  PORTB &= ~(1 << cs_pin);  // Pull CS low
  SPISend(reg);             // Send register address
  SPISend(data);            // Send data
  PORTB |= (1 << cs_pin);   // Pull CS high
}

// Initialize MAX7219
void MAX7219_Init() {
  MAX7219_Send(CS1, 0x0C, 0x01); // Exit shutdown mode for Matrix 1
  MAX7219_Send(CS2, 0x0C, 0x01); // Exit shutdown mode for Matrix 2
  //MAX7219_Send(CS3, 0x0C, 0x01); // Exit shutdown mode for Matrix 3
  
  MAX7219_Send(CS1, 0x09, 0x00); // Disable decode mode for Matrix 1
  MAX7219_Send(CS2, 0x09, 0x00); // Disable decode mode for Matrix 2
  //MAX7219_Send(CS3, 0x09, 0x00); // Disable decode mode for Matrix 3

  MAX7219_Send(CS1, 0x0B, 0x07); // Set scan limit to 8 digits for Matrix 1
  MAX7219_Send(CS2, 0x0B, 0x07); // Set scan limit to 8 digits for Matrix 2
  //MAX7219_Send(CS3, 0x0B, 0x07); // Set scan limit to 8 digits for Matrix 3
  
  MAX7219_Send(CS1, 0x0A, 0x08); // Set intensity for Matrix 1
  MAX7219_Send(CS2, 0x0A, 0x08); // Set intensity for Matrix 2
  //MAX7219_Send(CS3, 0x0A, 0x08); // Set intensity for Matrix 3

  MAX7219_Send(CS1, 0x0F, 0x00); // Disable display test for Matrix 1
  MAX7219_Send(CS2, 0x0F, 0x00); // Disable display test for Matrix 2
  //MAX7219_Send(CS3, 0x0F, 0x00); // Disable display test for Matrix 3
}

void rotateMatrix(const uint8_t input[8], uint8_t output[8], uint8_t rotations) {
  uint8_t temp[8];
  uint8_t intermediate[8];
  
  // Copy input into a working array
  for (uint8_t i = 0; i < 8; i++) {
      temp[i] = input[i];
  }

  // Perform the specified number of rotations
  for (uint8_t r = 0; r < rotations; r++) {
      for (uint8_t row = 0; row < 8; row++) {
          intermediate[row] = 0; // Clear the row
          for (uint8_t col = 0; col < 8; col++) {
              if (temp[col] & (1 << row)) {
                  intermediate[row] |= (1 << (7 - col));
              }
          }
      }

      // Copy the intermediate result back to temp for the next iteration
      for (uint8_t i = 0; i < 8; i++) {
          temp[i] = intermediate[i];
      }
  }

  // Copy the final rotated matrix to the output
  for (uint8_t i = 0; i < 8; i++) {
      output[i] = temp[i];
  }
}

// Function to get the bitmap of a character (A-Z, 0-9)
uint8_t* getCharacterBitmap(char character) {
    // Check if the character is in the valid range (A-Z or 0-9)
    if (character >= 'A' && character <= 'Z') {
        return (uint8_t*)font[character - 'A']; // Return pointer to the character bitmap for A-Z
    } else if (character >= '0' && character <= '9') {
        return (uint8_t*)font[26 + (character - '0')]; // Return pointer to the character bitmap for 0-9
    } else {
        // For unsupported characters, return a blank bitmap
        static uint8_t blank[8] = {0};
        return blank;
    }
}

// Function to load and process a string for display
void loadTextFromString(const char *str, uint8_t* buffer) {
  int string_length = strlen(str);
  for (int i = 0; i < string_length; i++) {
    uint8_t* charBitmap = getCharacterBitmap(str[i]); // Get the bitmap for the current character
    
    if(charBitmap != NULL){
      uint8_t rotatedChar[8];
      rotateMatrix(charBitmap, rotatedChar, 1);  // Rotate if necessary
      
      // Store the rotated character in the buffer
      for (int j = 0; j < 8; j++) {
        buffer[i * 8 + j] = rotatedChar[j];
      }
    }
  }
}

// Function to shift the entire text left by one column
void scrollText(uint8_t* buffer, uint8_t buffer_length) {
    uint8_t i;
    uint8_t length = buffer_length - 1;
    // Shift all columns in the buffer to the left by one
    for (i = 0; i < length; i++) {
        buffer[i] = buffer[i + 1];
    }
    buffer[length] = 0;  // Set the last column to zero
}

void displayText(uint8_t* buffer, uint8_t numMatrices) {
  for (uint8_t matrix = 0; matrix < numMatrices; matrix++) {
    for (uint8_t row = 0; row < 8; row++) {
      uint8_t rowData = 0;
      for (uint8_t col = 0; col < 8; col++) {
        if (buffer[(matrix * 8) + col] & (1 << row)) {
          rowData |= (1 << col);
        }
      }
      // Send data to the appropriate matrix (CS pin is passed dynamically)
      if (matrix == 0) {
        MAX7219_Send(CS1, row + 1, rowData);
      } else if (matrix == 1) {
        MAX7219_Send(CS2, row + 1, rowData);
      } else if (matrix == 2) {
        //MAX7219_Send(CS3, row + 1, rowData);
      }
    }
  }
}

int main(void)
{
  // Initialise SPI and drivers
  SPIInit();
  MAX7219_Init();

  // Example string to display
  const char* message = "HELLO";  // Your message to scroll or display
  uint8_t message_length = strlen(message);
  uint8_t buffer_length = 8 * message_length; 
  
  uint8_t buffer[buffer_length];
  loadTextFromString(message, buffer);  // Load the text and display it

  uint8_t num_matrices = 2;

  while (1)
  {
    // Shift the text by one column and display it
    scrollText(buffer, buffer_length);
    displayText(buffer, num_matrices);
    _delay_ms(100); // Adjust speed of scrolling
  }
}