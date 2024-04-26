/* driver adapted from github neopixel ws2812, modified by team studious */
#include <LED_ring.h>
#include <msp430.h>

//initialize a struct for grb data transmit
typedef struct {
    u_char green;
    u_char red;
    u_char blue;
} LED;


//led data array
static LED leds[NUM_LEDS] = { { 0, 0, 0 } };

// initialize spi ports interface
void initStrip() {
    P1SEL |= BIT2;    // configure output pin as SPI output
    P1SEL2 |= BIT2;
    UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC;
    UCA0CTL1 |= UCSSEL_2;   // SMCLK source (16 MHz)
    UCA0BR0 = 3;
    UCA0BR1 = 0;
    UCA0CTL1 &= ~UCSWRST;
    clearStrip();           // clear the strip
}

// Sets the color of a certain LED
void setLEDColor(u_int p, u_char r, u_char g, u_char b) {
	leds[p].green = g;
    leds[p].red = r;
    leds[p].blue = b;
}


// Send colors to the strip and show them.
void showStrip() {
    __bic_SR_register(GIE);  // disable interrupts

    // send RGB color for every LED
    unsigned int i, j;
    for (i = 0; i < NUM_LEDS; i++) {
        u_char *rgb = (u_char *)&leds[i]; // get GRB color for this LED

        // send g -> r -> b
        for (j = 0; j < 3; j++) {
            u_char mask = 0x80;

            // check each of the 8 bits
            while (mask != 0) {
                while (!(IFG2 & UCA0TXIFG))
                    ;    // wait to transmit
                if (rgb[j] & mask) {        // most significant bit first
                    UCA0TXBUF = HIGH_CODE;  // send 1
                } else {
                    UCA0TXBUF = LOW_CODE;   // send 0
                }

                mask >>= 1;  // check next bit
            }
        }
    }

    // delay 50 us
    _delay_cycles(800);

    __bis_SR_register(GIE);    // enable interrupts
}

// Clear the color of all LEDs (make them black/off)
void clearStrip() {
    fillStrip(0x00, 0x00, 0x00);
}

// Fill the strip with a solid color. This will update the strip.
void fillStrip(u_char r, u_char g, u_char b) {
    int i;
    for (i = 0; i < NUM_LEDS; i++) {
        setLEDColor(i, r, g, b);  // set all LEDs to specified color
    }
    showStrip();  // refresh strip
}
