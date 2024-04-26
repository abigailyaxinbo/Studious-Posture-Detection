
#define NUM_LEDS    (24)    // NUMBER OF LEDS IN YOUR STRIP

//typedefs easier for typing
typedef unsigned char u_char;	// 8 bit
typedef unsigned int u_int;     // 16 bit

// Transmit codes for showing color
#define HIGH_CODE   (0xF0)      // b11110000
#define LOW_CODE    (0xC0)      // b11000000

// Configure led ring
void initStrip(void);

// Send colors to the strip and show them.
void showStrip(void);

// Set the color of a certain LED
void setLEDColor(u_int p, u_char r, u_char g, u_char b);

// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b);

// Clear the color of all LEDs (make them black/off)
void clearStrip(void);


