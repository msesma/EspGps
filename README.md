# EspGps
ESP 32 and a GPS module: 
   Receive NMEA signals from a GPS and show speed in a tm1637 display

   Future development:
        Add road radar database and monitor distance to radars
        Signal radar presence.
        Show average speed for section radars
        Add a buzzer to signal when speed is over the limit
        Add a RGB led to signal states
        Add a way to update the radar database
        Add an LDR to control display and RGB led brightness

PIN usage for ESP32 C3 SuperMini
- GPS: UART 1, RX 20, TX 21 (Not used yet)
- RGB LED: Data 10
- Internal LED: 8
- 7 segment x 4 tm1637 display: DIO 0, CLK 1
- microSD SDI: MISO 5, MOSI 6, SS/CS 7, SCK 4

