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
  // -- sample command and data on END_TICK signal
  // 0x2 "0010xxxx aaaaaaaa aaaaaaaa aaaaaaaa" = set address command
  // 0x3 "0011xxxx aaaaaaaa aaaaaaaa aaaaaaaa" = set address command with autoinc after next read or write
  // 0x8 "1000xxxx xxxxxxxx xxxxxxxx dddddddd" = write data command
  // 0x4 "0100xxxx xxxxxxxx xxxxxxxx xxxxxxxx" = read data command
  // 0xC "1100xxxx xxxxxxxx xxxxxxxx xxxxxxxx" = read ID/version command
  // 0x0 "0000xxxx xxxxxxxx xxxxxxxx xxxxxxxx" = SPI read cycle, return data from last read command

  constexpr uint32_t setAddrCmd     = 0x20000000; // set address command
  constexpr uint32_t setAddrCmdAutoInc = 0x30000000; // set address command with autoinc after next read or write
  constexpr uint32_t writeDataCmd   = 0x80000000; // write data command
  constexpr uint32_t readDataCmd    = 0x40000000; // read data command
  constexpr uint32_t readCycle      = 0x00000000; // read data command
  constexpr uint32_t readVersionCmd = 0xC0000000; // read ID/version command
  constexpr uint32_t setChipType    = 0xA0000000; // set chip type command
  constexpr uint32_t SetResetCmd    = 0xF0000000; // set reset command
  constexpr uint32_t ClearResetCmd  = 0xE0000000; // clear reset command

  // Clears all output bits on the shift register, go to emulation mode
  void clearDataBus() {
  }

  void startBlockTransfer(uint32_t startAddr) {
    uint32_t txlong = startAddr | setAddrCmdAutoInc; // set address command (0x3) with autoinc after next read or write
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(txlong); // set address to read or write
    digitalWrite(LATCH_PIN, HIGH);
    delayMicroseconds(1); // wait for FPGA to process the command
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(SetResetCmd); // set address to read or write
    digitalWrite(LATCH_PIN, HIGH);
  }

  void stopBlockTransfer() {
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(setAddrCmd); // set address
    digitalWrite(LATCH_PIN, HIGH);
    delayMicroseconds(1); // wait for FPGA to process the command
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(ClearResetCmd); // set address to read or write
    digitalWrite(LATCH_PIN, HIGH);
  }

  uint32_t spi_xfer32(uint32_t data) {
    uint32_t rxlong;
    digitalWrite(LATCH_PIN, LOW);
    rxlong  = SPI.transfer16(data >> 16) << 16;
    rxlong |= SPI.transfer16(data & 0xFFFF);
    digitalWrite(LATCH_PIN, HIGH);
    return rxlong;
  }

  // Sends one byte to device output
  void outputByte(uint32_t value) {
    // Shifts one byte to SPI
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(value | writeDataCmd); // write command
    digitalWrite(LATCH_PIN, HIGH);
  }

  // receives one byte from device
  uint8_t inputByte() {
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(readDataCmd); // read command
    digitalWrite(LATCH_PIN, HIGH);
    uint32_t rxlong = spi_xfer32(readCycle) & 0xFF; // read back data from internal register
    return static_cast<uint8_t>(rxlong);
  }

  void getFPGAversion() {
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32(readVersionCmd); // read ID/version command
    digitalWrite(LATCH_PIN, HIGH);
    fpgaVersion = spi_xfer32(readCycle); // read back data from internal register
    Serial.printf("FPGA version: %08X\n", fpgaVersion);
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
    readyMessage();
    return true;
  }

  void testSPItransfer() {
    // Write test pattern to internal registers and read back to verify correctness.
    Serial.println(F("Testing SPI transfer to FPGA BRAM..."));
    dy1message(F("tst"));
    drawStringBox("Test", "Pattern", 0);
    uint8_t vals_written[16];
    uint32_t start_addr = 0x0400;
    startBlockTransfer(start_addr);
    for (uint32_t i = 0; i < 16; ++i) {
      vals_written[i] = static_cast<uint8_t>(0xA5 - i*9);
      outputByte(vals_written[i]);
    }
    startBlockTransfer(start_addr);
    for (uint32_t i = 0; i < 16; ++i) {
      // read back data from internal register
      Serial.print(F("Addr 0x"));
      Serial.print(start_addr + i, HEX);
      Serial.print(F(", Written 0x"));
      Serial.print(vals_written[i], HEX);
      Serial.print(F(", Received 0x"));
      Serial.println(inputByte(), HEX);
    }
    stopBlockTransfer();
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
void outputChipType(uint32_t value) {
  #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
    digitalWrite(LATCH_PIN, LOW);
    SPI.write32((value & 0x0F) | setChipType); // set chip type command
    digitalWrite(LATCH_PIN, HIGH);
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
  readyMessage();
}
