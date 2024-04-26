#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <LED_ring.h>
/**
 * main.c
 */
//acceleration registers
#define SET_ACC 0x10
#define SET_GYRO 0x11
#define X_GYRO_L 0x22
#define X_GYRO_H 0x23
#define Y_GYRO_L 0x24
#define Y_GYRO_H 0x25
#define Z_GYRO_L 0x26
#define Z_GYRO_H 0x27
#define X_ACC_L 0x28
#define X_ACC_H 0x29
#define Y_ACC_L 0x2A
#define Y_ACC_H 0x2B
#define Z_ACC_L 0x2C
#define Z_ACC_H 0x2D

//prototype
void ser_output(char *str);
void itoa(int n, char s[]);
void reverse(char s[]);
int read_IMU_SPI(uint8_t register_address, unsigned int chip_select);
int send_IMU_SPI(uint8_t register_address, uint8_t write_byte, unsigned int chip_select);
void setup_IMU_SPI(void);
//void gradualFill(u_int n, u_char r, u_char g, u_char b);

char mid[] = ", ";
char charreturn[] = "\r\n";
char line[] = "The value is ";

//bound for outliers because of sign change
int bound = 3500;

//read value from imu
volatile int16_t x_gyro;
volatile int16_t y_gyro;
volatile int16_t z_gyro;
volatile int16_t x_acc;
volatile int16_t y_acc;
volatile int16_t z_acc;

//current avg data
volatile int16_t avg1;
volatile int16_t avg2;
volatile int16_t avg3;
volatile int16_t avg4;
volatile int16_t avg5;
volatile int16_t avg6;

//saved avg data
volatile int16_t preavg1;
volatile int16_t preavg2;
volatile int16_t preavg3;
volatile int16_t preavg4;
volatile int16_t preavg5;
volatile int16_t preavg6;

//change in data
volatile int16_t davg1; //x_g
volatile int16_t davg2; //y_g
volatile int16_t davg3; //z_g
volatile int16_t davg4; //x_a
volatile int16_t davg5; //y_a
volatile int16_t davg6; //z_a

//total for each running average
volatile int16_t total1;
volatile int16_t total2;
volatile int16_t total3;
volatile int16_t total4;
volatile int16_t total5;
volatile int16_t total6;

//variables for running average
int numReadings = 10;
volatile int16_t readings1[10];
volatile int16_t readings2[10];
volatile int16_t readings3[10];
volatile int16_t readings4[10];
volatile int16_t readings5[10];
volatile int16_t readings6[10];


//flags for alarms
int flag_l = 0; //lower threshold
int flag_h = 0; //higher threshold

int i = 0;
int readIndex = 0;


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    setup_IMU_SPI();
    initStrip();

    send_IMU_SPI(SET_ACC, 0x38, BIT3);  //2g 52hz
    send_IMU_SPI(SET_GYRO, 0x34, BIT3);  //500dps 52hz

    while(1)
    {
        preavg1 = avg1;
        preavg2 = avg2;
        preavg3 = avg3;
        preavg4 = avg4;
        preavg5 = avg5;
        preavg6 = avg6;

        /* beginning of running average */
        total1 -= readings1[readIndex];
        total2 -= readings2[readIndex];
        total3 -= readings3[readIndex];
        total4 -= readings4[readIndex];
        total5 -= readings5[readIndex];
        total6 -= readings6[readIndex];
        //read data
        x_gyro = (read_IMU_SPI(X_GYRO_H, BIT3) << 8) | read_IMU_SPI(X_GYRO_L, BIT3);
        y_gyro = (read_IMU_SPI(Y_GYRO_H, BIT3) << 8) | read_IMU_SPI(Y_GYRO_L, BIT3);
        z_gyro = (read_IMU_SPI(Z_GYRO_H, BIT3) << 8) | read_IMU_SPI(Z_GYRO_L, BIT3);
        x_acc = (read_IMU_SPI(X_ACC_H, BIT3) << 8) | read_IMU_SPI(X_ACC_L, BIT3);
        y_acc = (read_IMU_SPI(Y_ACC_H, BIT3) << 8) | read_IMU_SPI(Y_ACC_L, BIT3);
        z_acc = (read_IMU_SPI(Z_ACC_H, BIT3) << 8) | read_IMU_SPI(Z_ACC_L, BIT3);
        //store data in each running average array
        readings1[readIndex] = x_gyro;
        readings2[readIndex] = y_gyro;
        readings3[readIndex] = z_gyro;
        readings4[readIndex] = x_acc;
        readings5[readIndex] = y_acc;
        readings6[readIndex] = z_acc;

        //shift data range and take average
        total1 += readings1[readIndex];
        total2 += readings2[readIndex];
        total3 += readings3[readIndex];
        total4 += readings4[readIndex];
        total5 += readings5[readIndex];
        total6 += readings6[readIndex];

        readIndex += 1;

        if (readIndex >= numReadings){
            readIndex = 0;
        }

        avg1 = total1 / numReadings;
        avg2 = total2 / numReadings;
        avg3 = total3 / numReadings;
        avg4 = total4 / numReadings;
        avg5 = total5 / numReadings;
        avg6 = total6 / numReadings;
        /* end of running average */

        //calculate the change in acceleration and angular rate
        davg1 = preavg1 - avg1;
        davg2 = preavg2 - avg2;
        davg3 = preavg3 - avg3;
        davg4 = preavg4 - avg4;
        davg5 = preavg5 - avg5;
        davg6 = preavg6 - avg6;

        //get rid of outliers
        if (davg1 > bound || davg1 < (-1) * bound || davg2 < (-1) * bound || davg2 > bound ||
            davg3 < (-1) * bound || davg3 > bound || davg4 > bound || davg4 < (-1) * bound ||
            davg5 < (-1) * bound || davg5 > bound || davg6 > bound || davg6 < (-1) * bound)
        {
            //ser_output("outlier");
            continue;
        }

        //test triggers
        if (abs(davg1) > 500 || abs(davg2) > 300 || abs(davg3) > 1000 ||
            abs(davg4) > 1000 || abs(davg5) > 600 || abs(davg6) > 300)
        {
            flag_l = 1;
            flag_h = 0;
            if (abs(davg1) > 1000 || abs(davg2) > 500 || abs(davg3) > 1500 ||
                abs(davg4) > 1300 || abs(davg5) > 900  || abs(davg6) > 500)
            {
                flag_l = 1;
                flag_h = 1;
            }
        }
        else
        {
            flag_l = 0;
            flag_h = 0;
        }

        if (i < NUM_LEDS) {
            if (flag_l == 1 && flag_h == 0) {
                setLEDColor(i, 0x08, 0x08, 0x00); //yellow
            }
            else if (flag_l == 1 && flag_h == 1) {
                setLEDColor(i, 0x0A, 0x00, 0x00); //red
            }
            else {
                setLEDColor(i, 0x00, 0x0A, 0x00); //green
            }
            showStrip();
            i++;
        } else {
            i = 0;
        }
        __delay_cycles(500000); //0.5M cycles -> 5/160 second
    }
    return 0;
}


void setup_IMU_SPI(void){

    DCOCTL = 0;                 // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_16MHZ;     // Set range
    DCOCTL = CALDCO_16MHZ;      // Set DCO step + modulation */
    BCSCTL3 |= LFXT1S_2;        // ACLK = VLO - This is also called in the init_wdt() function
    WDTCTL = WDT_ADLY_16;    // WDT 16ms (~43.3ms since clk 12khz), ACLK, interval timer

    P2DIR |= BIT0 + BIT3 + BIT4 + BIT5;  //only 2.3,4,5 can be used for chipselect

    //COPI on p1.7, SCLK on p1.5
    P1SEL |= BIT5 + BIT6 + BIT7;
    P1SEL2 |= BIT5 + BIT6 + BIT7;

    UCB0CTL1=UCSWRST; //disable serial interface
    UCB0CTL0 |= UCCKPL + UCMSB + UCMST + UCSYNC;    // data polarity, MSB first, master mode, synchronous
    UCB0CTL1 |= UCSSEL_2;                           // select SMCLK
    UCB0BR0 = 0x10;                                    //set frequency
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;           // Initialize USCI state machine

}

int read_IMU_SPI(uint8_t register_address, unsigned int chip_select){
    uint8_t dataOut = 0;

    P2OUT &= ~chip_select;
    __delay_cycles(20);

    UCB0TXBUF = register_address | 0x80;
    while(!(IFG2 & UCB0TXIFG)); // Wait until TX finished
    UCB0TXBUF = 0x00;
    while(!(IFG2 & UCB0TXIFG)); // Wait until TX finished
    dataOut = UCB0RXBUF;
    while (!(IFG2 & UCB0RXIFG));
    dataOut = UCB0RXBUF;

    P2OUT |= chip_select;
    __delay_cycles(100);
    return dataOut;
}

int send_IMU_SPI(uint8_t register_address, uint8_t write_byte, unsigned int chip_select) {
    uint8_t dataOut = 0;

    P2OUT &= ~chip_select;
    _delay_cycles(20);
    //write the address you want to read
    UCB0TXBUF = register_address;
    while(!(IFG2 & UCB0TXIFG)); // Wait until TX finished
    UCB0TXBUF = write_byte;
    while(!(IFG2 & UCB0TXIFG)); // Wait until TX finished

    dataOut = UCB0RXBUF;
    while (!(IFG2 & UCB0RXIFG));
    dataOut = UCB0RXBUF;

    P2OUT |= chip_select;
    __delay_cycles(100);
}



