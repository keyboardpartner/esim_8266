#pragma once

#include <Arduino.h>

// ################################################################################
//
//      #####  ######  ###       #     # ####### ####### ######  
//     #     # #     #  #         #   #  #       #       #     # 
//     #       #     #  #          # #   #       #       #     # 
//      #####  ######   #           #    #####   #####   ######  
//           # #        #          # #   #       #       #   #   
//     #     # #        #         #   #  #       #       #    #  
//      #####  #       ###       #     # #       ####### #     # 
//                                                               
// ################################################################################
// SPI Transfer functions for various devices (GODIL, MOJO FPGA, ESIM, PEPS)
// ################################################################################

// Controls the onboard LED used as HTTP upload activity indicator.
void setUploadLed(bool on) {
  #ifndef JTAG_SPARTAN6
    digitalWrite(LED_UPLOAD, on ? HIGH : LOW);
  #endif
}

// Controls a dedicated LED that indicates active file playback/streaming.
void setUpSendLed(bool on) {
  #ifndef JTAG_SPARTAN6
    digitalWrite(LED_SENDDATA, on ? HIGH : LOW);
  #endif
}

#if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)

  #define SS_HIGH  GPOS = (1<<LATCH_PIN)
  #define SS_LOW   GPOC = (1<<LATCH_PIN)

  // x"00 AA AA AA" = set address command
  // x"02 xx xx DD" = write RAM data command
  // x"03 xx xx DD" = write RAM data command with autoinc after next write
  // x"04 xx xx xx" = read RAM data command
  // x"05 xx xx xx" = read RAM data command with autoinc after next read
  // x"08 xx xx xx" = read ID/version command
  // x"0A xx xx xx" = Set Type Select (CFG output nibble)
  // x"0F xx xx xB" = Set Reset line to Bit 0, B = 0 oder 1
  // x"4P xx xx DD" = Write Port P = 0..3

  constexpr uint8_t setAddrCmd      = 0x00; // set address command
  constexpr uint8_t writeDataCmd    = 0x02; // write RAM data command
  constexpr uint16_t writeDataIncCmd = 0x03; // write RAM data command with autoinc after next write
  constexpr uint8_t readDataCmd     = 0x04; // read RAM data command
  constexpr uint8_t readDataIncCmd  = 0x05; // read RAM data command
  constexpr uint8_t readVersionCmd  = 0x08; // read ID/version command
  constexpr uint8_t resetLineCmd    = 0x0F; // set reset line command
  constexpr uint16_t setChipTypeCmd = 0x0A; // set chip type command
  constexpr uint16_t setPortCmd     = 0x40; // set port command (0x40..0x4F)


  // Clears all output bits on the shift register, go to emulation mode
  void clearDataBus() {
  }

  void startBlockTransfer(uint32_t startAddr) {
    SS_LOW;
    SPI.write(resetLineCmd); // set address to read or write
    SPI.write(1); // set reset line bit to 1 (active)
    SS_HIGH;
    delayMicroseconds(1); // wait for FPGA to process the command
    SS_LOW;
    SPI.write(setAddrCmd); // set address command
    SPI.write32(startAddr); // set address to read or write
    SS_HIGH;

  }

  void stopBlockTransfer() {
    SS_LOW;
    SPI.write(resetLineCmd);
    SPI.write(0); // set reset line bit to 0 (inactive)
    SS_HIGH;
  }

  // Sends one byte to device output
  void outputByte(uint8_t value) {
    // Shifts one byte to SPI
    SS_LOW;
    SPI.write16(writeDataIncCmd << 8 | value); // write data command with autoinc after write
    // SPI.write(value); // write data command
    SS_HIGH;
  }

  // receives one byte from device
  uint8_t inputByte() {
    SS_LOW;
    SPI.write(readDataIncCmd); // read command with autoinc after read
    uint8_t rxbyte = SPI.transfer(0); // read back data from internal register
    SS_HIGH;
    return rxbyte;
  }

  // sends one 32 bit word to device
  void outputWord32(uint8_t command, uint32_t data) {
    SS_LOW;
    SPI.write(command); // send command
    SPI.write32(data);  // send data
    SS_HIGH;
  }

  // receives one 32 bit word from device
  uint32_t inputWord32(uint8_t command) {
    SS_LOW;
    SPI.write(command); // read command with autoinc after read
    uint32_t rxlong;
    rxlong  = SPI.transfer16(0) << 16;
    rxlong |= SPI.transfer16(0);
    SS_HIGH;
    return rxlong;
  }

  uint32_t getFPGAversion() {
    return inputWord32(readVersionCmd);
  }

  // Reads bytes from the slave device and stores them into a LittleFS file.
  bool writeEPROMtoFile(String filename, uint32_t start_addr, uint16_t len) {
    if (!fsMounted) {
      Serial.println(F("ERROR: filesystem not mounted."));
      return false;
    }

    filename.trim();
    if (filename.length() == 0) {
      Serial.println(F("ERROR: empty filename."));
      return false;
    }

    if (filename[0] != '/') {
      filename = String('/') + filename;
    }

    if (filename.indexOf("..") >= 0) {
      Serial.println(F("ERROR: invalid filename."));
      return false;
    }

    if (filename.length() > kMaxFsPathLength) {
      Serial.println(F("ERROR: filename too long."));
      return false;
    }

    if (len == 0) {
      Serial.println(F("ERROR: len must be > 0."));
      return false;
    }

    dy1message(F("dep"));
    drawStringBox("Dump", "RAM/EPROM", 0);
    LittleFS.remove(filename);
    File outFile = LittleFS.open(filename, "w");
    if (!outFile) {
      Serial.print(F("ERROR: cannot open file for writing: "));
      Serial.println(filename);
      return false;
    }
    startBlockTransfer(start_addr);
    for (uint32_t i = 0; i < len; ++i) {
      const uint8_t value = inputByte();
      if (outFile.write(&value, 1) != 1) {
        outFile.close();
        LittleFS.remove(filename);
        Serial.print(F("ERROR: write failed at offset "));
        Serial.println(i);
        return false;
      }
      if ((i & 0x1F) == 0) {
        delay(0); yield();
      }
    }
    stopBlockTransfer();
    outFile.close();
    Serial.print(F("EPROM dump saved to "));
    Serial.print(filename);
    Serial.print(F(", bytes="));
    Serial.println(len);
    return true;
  }

  void testSPItransfer() {
    // Write test pattern to internal registers and read back to verify correctness.
    Serial.println(F("Testing SPI transfer to FPGA BRAM..."));
    dy1message(F("tst"));
    drawStringBox("Test", "Pattern");
    uint8_t vals_written[16], vals_read[16];
    uint32_t start_addr = 0x0400;
    startBlockTransfer(start_addr);
    for (int i = 0; i < 16; ++i) {
      vals_written[i] = static_cast<uint8_t>(0xA5 - i*9);
      outputByte(vals_written[i]);
    }
    startBlockTransfer(start_addr);
    for (int i = 0; i < 16; ++i) {
      vals_read[i] = inputByte();
    }
    stopBlockTransfer();
    for (int i = 0; i < 16; ++i) {
      // read back data from internal register
      Serial.print(F("Addr 0x"));
      Serial.print(start_addr + i, HEX);
      Serial.print(F(", Written 0x"));
      Serial.print(vals_written[i], HEX);
      Serial.print(F(", Received 0x"));
      Serial.println(vals_read[i], HEX);
    }
  }
#endif

#ifdef ESIM_SPI
  // Clears all output bits on the shift register.
  void clearDataBus() {
    digitalWrite(STROBE_PIN, HIGH);
    digitalWrite(LATCH_PIN, LOW);
    SPI.transfer(0);
    digitalWrite(LATCH_PIN, HIGH);
  }

  void startBlockTransfer(uint32_t startAddr) {
  }
  void stopBlockTransfer() {
  }

  // Pulses the external strobe signal once.
  void pulseStrobe() {
    // The 74HC595 output update happens on latch edge in setDataBus().
    delayMicroseconds(strobeDelayMicros);
    digitalWrite(STROBE_PIN, LOW);
    delayMicroseconds(strobeDelayMicros);
    digitalWrite(STROBE_PIN, HIGH);
  }

  // Sends one byte to outputs and applies the configured inter-byte delay.
  void outputByte(uint8_t value) {
    // Shifts one byte to the 74HC595 using SPI and latches the new output state.
    digitalWrite(LATCH_PIN, LOW);
    SPI.transfer(value);
    digitalWrite(LATCH_PIN, HIGH);
    pulseStrobe();
    delayMicroseconds(strobeDelayMicros);
  }

  void testSPItransfer() {
    Serial.println(F("Testing SPI transfer, sending fixed Byte 0x35."));
    Serial.println(F("Press any key to stop test."));
    dy1message(F("tst"));
    drawStringBox("Test", "Pattern");
    clearDataBus();
    do {
      outputByte(0x35);
      delay(1);
    } while (not Serial.available());
    Serial.println(F("Ready."));
  }
#endif

#ifdef PEPS_SPI
  // MODE 3, sets address counter to 0
  void clearDataBus() {
  }

  void pepsMode3() {
    digitalWrite(M0_PIN, HIGH);
    digitalWrite(M1_PIN, HIGH);
  }


  void clkPulse() {
    digitalWrite(LATCH_CLK, HIGH);
    digitalWrite(LATCH_CLK, LOW);
  }


  void pepsMode0() {
    digitalWrite(M0_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
  }

  void pepsReset() {
    digitalWrite(DATA_PIN, LOW);
    digitalWrite(LATCH_CLK, LOW);
    pepsMode0();
    delayMicroseconds(1);
    pepsMode3();
  }

  void pepsInc() {
    pepsMode0();
    clkPulse();
    pepsMode3();
  }

  void startBlockTransfer(uint32_t startAddr) {
    SPI.end(); // we use bit-banging for PEPS SPI, so disable hardware SPI
    pinMode(LATCH_CLK, OUTPUT);
    digitalWrite(LATCH_CLK, LOW);
    pinMode(DATA_PIN, OUTPUT);
    pepsReset();
    if (startAddr > 0) {
      startAddr--;
      for (uint32_t i = 0; i < startAddr; i++) {
        pepsInc();
      }
    }
  }

  void stopBlockTransfer() {
    pepsMode3();
    digitalWrite(LATCH_CLK, HIGH);
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setFrequency(4000000);
  }

  // Sends one byte to outputs and applies the configured inter-byte delay.
  void outputByte(uint8_t value, uint32_t addr) {
    // Shifts one byte to the 74HC595 using SPI and latches the new output state.
    digitalWrite(DATA_PIN, LOW);
    digitalWrite(M1_PIN, LOW);
    for (int8_t i = 7; i >= 0; i--) {
      #ifdef DATA_INVERT
        digitalWrite(DATA_PIN, !((value >> i) & 0x01));
      #else
        digitalWrite(DATA_PIN, (value >> i) & 0x01);
      #endif
      clkPulse();
    }
    digitalWrite(DATA_PIN, LOW);
    digitalWrite(M1_PIN, HIGH);
    digitalWrite(M0_PIN, LOW);
    clkPulse(); // Write Byte
    pepsInc();
  }

  void testSPItransfer() {
    Serial.println(F("Testing SPI transfer, sending fixed Byte 0x35."));
    Serial.println(F("Press any key to stop test."));
    dy1message(F("tst"));
    drawStringBox("Test", "Pattern");
    startBlockTransfer(0);
    do {
      outputByte(0x35, 0);
      delayMicroseconds(100);
    } while (not Serial.available());
    stopBlockTransfer();
    Serial.read(); // clear any pending input
  }
#endif

// Sends chip type ID (0..15) to device output for GODIL/JTAG modes.
// Other modes ignore this setting.
void outputChipType(uint8_t value) {
  #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
    SS_LOW;
    SPI.write16(setChipTypeCmd << 8 | value); // write data command with autoinc after write
    SS_HIGH;
  #else
    (void)value;
  #endif
}

void outputPort(int port, uint8_t value) {
  #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
    SS_LOW;
    uint16_t set_port_cmd = setPortCmd | (port & 0x0F); // set port command (0x40..0x4F)
    SPI.write16(set_port_cmd << 8 | value); // write data command with autoinc after write
    SS_HIGH;
  #else
    (void)value;
  #endif
}

void eraseEPROMsilent() {
  startBlockTransfer(0);
  for (uint32_t i = 0; i < kMaxBytesToTransfer[currentChipTypeIndex]; ++i) {
    outputByte(0xFF);
    if ((i & 0xFF) == 0) {
      delay(0); yield();
    }
  }
  stopBlockTransfer();
}

void eraseEPROM() {
  Serial.print(F("Erasing EPROM... "));
  dy1message(F("Ers"));
  drawStringBox("Erase", "RAM/EPROM", 0);
  eraseEPROMsilent();
  Serial.println(F("Done."));
}
