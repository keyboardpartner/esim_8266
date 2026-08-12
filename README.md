# EPROM Simulator Uploader

### ESP8266 Project for PlatformIO or Arduino IDE

<img src="/docs/Screenshot_web_interface.png" width="320">

For development of legacy computer systems and embedded controllers, an EPROM simulator is mandatory. Unfortunately, they are no longer popular and availability is sparse, while older models lack an appropriate interface. I made a few versions shown heere. Set *#defines* in *main.cpp* accordingly.

### ESIM Support

<img src="https://github.com/keyboardpartner/esim_8266/blob/main/docs/esim.jpg" width="320">

ESP8266 used as an upload tool for old ESIM EPROM Simulator from c't magazine, issue 9/1991 instead of obsolete Centronix port. Outputs stream of data to 74HC595 shift register. Pulls /STROBE low on each byte sent. Supports 2764 up tp 27512 EPROMs. Unfortunately, ESIM does not support setting a start address; uploads will always start at EPROM address 0x0000.

#### Pinout ESIM

Centronix Sub-D-25 for IBM PC, box pin header, 26 pins. Sub-D-25 pins in brackets

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



### CONITEC PEPS Support

New: now supports even older PEPS EPROM Simulator from c't 5/1985, page 85. This design uses a special serial transmission; due to the protocol's nature, the byte output routine uses slower bit-banging instead of SPI. EPROM size is limited to 27128 (16 KBytes). Find the old article in /docs folder.

### GODIL FPGA Support

<img src="https://github.com/keyboardpartner/esim_8266/blob/main/docs/godil.jpg" width="320">

I made a versatile EPROM simulator using the legacy GODIL 40 from [OHO electronic](https://www.oho-elektronik.de/). See my other repositories. These are legacy FPGA modules sold by [Trenz electronic](https://www.trenz-electronic.de/de), but you may still find them on the web. Supports EPROMs up to 27256 (32 KByte). 

This version has a nice feature: It can be used as RAM emulator for 6264 or 62256 SRAMs; in this case, you may read back the RAM contents from host system. Used it to recover old programs from my very first computer, an Acorn ATOM from 1981, using the rom_adapter_2 circuit. See /pcb folder. 

### MOJO V3 FPGA Support

<img src="https://github.com/keyboardpartner/esim_8266/blob/main/docs/mojo.jpg" width="320">

(Under construction)

Using a MOJO V3 FPGA board as EPROM simulator allows for 64 KByte EPROMs and SRAMs, merely by using internal FPGA BRAMs.

MOJO is a FPGA board from embeddedmicro.com, formerly advertized as "open source". Unfortunately, the manufacturer ceased operation, and documentation is sparse. This is how not to do "open source". However, some cheap rip-offs are still available through Aliexpress. The MOJO design is rather bad, using an AVR for (slowly!) uploading the configuration bitstream from an SPI flash via Master SelectMAP (instead of attaching the SPI flash direcly to FPGA; by blocking the FPGA, flashing would have been possible as well).

I once made a [MOJO BIT file Uploader for Windows](https://github.com/keyboardpartner/MOJO_uploader) that flashes the SPI memory, but here a different approach is used: I send the FPGA configuration through JTAG (pins available on MOJO SV1). So the FPGA configuration may be uploaded and selected by web interface on-the-fly. The AVR on MOJO board is blocked by permanent reset. My fast JTAG loader sends the 334 KByte bitstream file for FPGA XILINX XC6SLX9 from LittleFS in about 920 ms (instead of 3.4 seconds of known version from RSP, see below), which is even faster than the ill-fated MOJO approach.

For MOJO-like boards with Xilinx XC9SLX9 FPGA, ensure the correct bitstream (default "fpga_main.bit") is uploaded to file system. The bitstream is loaded into the FPGA during startup configuration via JTAG.

This version can also be used as RAM emulator for 6264 or 62256 SRAMs, like the GODIL version. 


### OHO DY1 Support

All modules support the OHO DY1 display, a 3-digit 7-segment LED module with SPI connection, handy for displaying short messages, IP numbers etc. You may omit the DY1 display and use serial monitoring instead (to get your IP number and to set password/SSID).

### PCBs

For each version, I designed a one-layer PCB optimized for milling. Can be handled by JCLPCB, though. There are also designs for ROM/RAM adaptors. See /pcb folder. 

PCB design done with EasyPC from NumberOne Systems.

Useful links:

[MOJO v3 Bootloader and firmware](https://github.com/embmicro/mojo-bootloader/blob/master/Caterina.c)

[JTAG Player for ESP8266](https://github.com/rsp-esl/esp8266_jtag_fpga) (slow, as mentioned above)