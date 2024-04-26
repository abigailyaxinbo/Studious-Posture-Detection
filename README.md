# Studious-Posture-Detection

A workout assisting posture detection armband running on MSP430G2553 using SPI communication

Hardware List:
MSP430G2553 x 1
ISM330DLC 6-axis IMUx 1
24-WS2813 Mini RGB LED Ring x 1

# The program will:
1. Configure the board, the accelerometer, and the LED ring
2. Take input from accelerometer
3. Do data analysis
  a. Take the running average of the latest 10 read value of each measurement
  b. Take the difference between the current average and the previous average value
  c. Evaluate the user’s posture based on the magnitude of the difference
4. Output the color warning the user if they are doing exercises improperly, otherwise, keep the green LEDs on
5. Repeat step 2-4

# Block Diagram for General Process
![](https://github.com/abigailyaxinbo/Studious-Posture-Detection/blob/main/block%20diagram.png)

# To build the hardware:
1. Connect the following pins:
  a. LED Ring: 5V for VDD, GND for GND, 1.2 for SIG(or DIN)
  b. ISM330DLC: 3.3V for VDD, GND for GND, P1.5 for SCL, P1.7 for SDA, P1.6 for DO, P2.3 for CS
2. Build the band that: 
  a. Can hold MSP and LED band
  b. Place the accelerometer close to your wrist, on the upper side, face to the top.

# To use the code:
Put the electronic parts together
Make an armband to hold parts
Compile and flash the program into the microcontroller
Do the workout and observe the feedback on the LED ring!

Demo:

