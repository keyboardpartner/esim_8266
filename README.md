# ESIM EPROM Simulator Uploader

![PCB Pic](https://github.com/keyboardpartner/esim_8266/blob/main/docs/esim_esp_reversed.jpg)

### ESP8266 Project for PlatformIO or Arduino IDE

ESP8266 used as an upload tool for old ESIM EPROM Simulator from c't 9/1991 instead of obsolete Centronix port. Outputs stream of data to 74HC595 shift register. Pulls /STROBE low on each byte sent.

#### Pinout ESIM

Centronix Sub-D-25 for IBM PC, box pin header, 26 pins

| PFS-26 Pin | Sub-D-25 Pin | Signal     | PFS-26 Pin | Sub-D-25 Pin | Signal   |
| --- | --- | --- | --- | --- | --- |
| 1  | (1)  | /STB       | 2  | (14) | nc      |
| 3  | (2)  | D0         | 4  | (15) | nc      |
| 5  | (3)  | D1         | 6  | (16) | nc      |
| 7  | (4)  | D2         | 8  | (17) | nc      |
| 9  | (5)  | D3         | 10 | (18) | GND     |
| 11 | (6)  | D4         | 12 | (19) | GND     |
| 13 | (7)  | D5         | 14 | (20) | GND     |
| 15 | (8)  | D6         | 16 | (21) | GND     |
| 17 | (9)  | D7         | 18 | (22) | GND     |
| 19 | (10) | nc (ACK)   | 20 | (23) | GND     |
| 21 | (11) | GND (BUSY) | 22 | (24) | GND     |
| 23 | (12) | nc         | 24 | (25) | GND     |
| 25 | (13) | VCC-Host   | 26 | (-)  | VCC-Host |