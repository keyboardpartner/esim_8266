#pragma once

// ##############################################################################
//
//      #####  ####### ######  ###    #    #           #####  #     # ######  
//     #     # #       #     #  #    # #   #          #     # ##   ## #     # 
//     #       #       #     #  #   #   #  #          #       # # # # #     # 
//      #####  #####   ######   #  #     # #          #       #  #  # #     # 
//           # #       #   #    #  ####### #          #       #     # #     # 
//     #     # #       #    #   #  #     # #          #     # #     # #     # 
//      #####  ####### #     # ### #     # #######     #####  #     # ######  
//                                                                                    
// ##############################################################################
// SERIAL COMMANDS
// ##############################################################################


// ##############################################################################
//
//     ### #     # ####### #######    #     # ####### #       ######  
//      #  ##    # #       #     #    #     # #       #       #     # 
//      #  # #   # #       #     #    #     # #       #       #     # 
//      #  #  #  # #####   #     #    ####### #####   #       ######  
//      #  #   # # #       #     #    #     # #       #       #       
//      #  #    ## #       #     #    #     # #       #       #       
//     ### #     # #       #######    #     # ####### ####### #       
//                                                                                  
// ##############################################################################
// SERIAL INFO AND HELP TEXT
// ##############################################################################


void printDivLine() {
  Serial.println(F("-----------------------"));
}

void printIDcode() {
  Serial.println("");
  Serial.println(F("------ FPGA Info ------"));
  Serial.printf("ID code: %08X\n", jtagReadIDcode() );
  Serial.print(F("(X4001093 for Xilinx XC6SLX9)\n"));
  getFPGAversion();
  printDivLine();
}

void printFileInfo() {
  const bool stagedExists = fsMounted && currentFilePath.length() > 0 && LittleFS.exists(currentFilePath);
  Serial.println("");
  Serial.println(F("------ File Info ------"));
  Serial.print(F("fsMounted: "));
  Serial.println(fsMounted ? F("true") : F("false"));
  Serial.print(F("stagedExists: "));
  Serial.println(stagedExists ? F("true") : F("false"));
  Serial.print(F("stagedPath: "));
  Serial.println(currentFilePath.length() ? currentFilePath : String(F("none")));
  Serial.print(F("lastUploadStartAddr: "));
  Serial.println(lastUploadStartAddr);
  Serial.print(F("lastFilename: "));
  Serial.println(lastFilename.length() ? lastFilename : String(F("none")));
  Serial.print(F("lastFileBytes: "));
  Serial.println(lastFileBytes);
  printDivLine();
}

// Lists all LittleFS root directory entries on serial.
void listLittleFsEntries() {
  Serial.println("");
  if (!fsMounted) {
    Serial.println(F("ERROR: filesystem not mounted."));
    return;
  }
  Dir dir = LittleFS.openDir("/");
  size_t entryCount = 0;
  size_t totalBytes = 0;

  Serial.println(F("---- Directory Info ----"));
  while (dir.next()) {
    const String path = normalizeFsPath(dir.fileName());
    const size_t bytes = dir.fileSize();
    Serial.print(path);
    Serial.print(F(" ("));
    Serial.print(bytes);
    Serial.println(F(" bytes)"));
    ++entryCount;
    totalBytes += bytes;
    delay(0); yield();
  }
  if (entryCount == 0) {
    Serial.println(F("  <empty>"));
  }
  Serial.print(F("Entries: "));
  Serial.print(entryCount);
  Serial.print(F(", total bytes: "));
  Serial.println(totalBytes);
  printDivLine();
}

void printWebInfo() {
  Serial.println("");
  Serial.println(F("------ WiFi Info ------"));
  Serial.print(F("Wi-Fi mode: "));
  Serial.println(wifiModeLabel);
  Serial.print(F("Wi-Fi STA SSID: "));
  Serial.println(staSsid.length() ? staSsid : String(F("<empty>")));
  Serial.print(F("IP address: "));
  Serial.println(wifiIpAddress);
  Serial.print(F("AP SSID: "));
  Serial.println(kApSsid);
  printDivLine();
}


// ##############################################################################
//
//     #     # ####### #       ######  ####### ######   #####  
//     #     # #       #       #     # #       #     # #     # 
//     #     # #       #       #     # #       #     # #       
//     ####### #####   #       ######  #####   ######   #####  
//     #     # #       #       #       #       #   #         # 
//     #     # #       #       #       #       #    #  #     # 
//     #     # ####### ####### #       ####### #     #  #####  
//                                                                                    
// ##############################################################################
// SERIAL COMMAND HELPER FUNCTIONS
// ##############################################################################
                                                          

void hexdumpInputBytes256(uint32_t startAddr) {
  #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
    startBlockTransfer(startAddr);
    for (uint16_t rowStart = 0; rowStart < 256; rowStart += 16) {
      char line[96];
      int lineLen = snprintf(line, sizeof(line), "%08lX: ", static_cast<unsigned long>(startAddr + rowStart));
      for (uint8_t i = 0; i < 16; ++i) {
        const uint8_t value = inputByte();
        lineLen += snprintf(line + lineLen, sizeof(line) - lineLen, "%02X ", value);
      }

      if (lineLen > 0 && line[lineLen - 1] == ' ') {
        line[lineLen - 1] = '\0';
      }
      Serial.println(line);

      if ((rowStart & 0x007F) == 0) {
        delay(0); yield();
      }
    }
    stopBlockTransfer();
  #else
    Serial.println(F("ERROR: command j is only available in GODIL or JTAG mode."));
  #endif
}

void printSerialCommandsInfo() {
  Serial.println("");
  Serial.println(F("--- Serial commands ---"));
  #ifdef JTAG_SPARTAN6
    Serial.println(F("c: config FPGA with last/staged file"));
  #endif
  Serial.println(F("d<start><CR>: hexdump 256 bytes from device"));
  Serial.println(F("e: erase EPROM"));
  Serial.println(F("h: print web/status info and help text (this one)"));
  Serial.println(F("i: print file debug info"));
  Serial.println(F("l: list LittleFS directory entries"));
  Serial.println(F("t: test SPI transfer"));
  Serial.println(F("r: replay last/staged file (blocking)"));
  Serial.println(F("-----------------------"));
  Serial.println(F("j<filename,start,len><CR>: dump EPROM to file"));
  Serial.println(F("n<filename><CR>: set next serial upload filename"));
  Serial.println(F("u<lenLo><lenHi><data...><cksLo><cksHi>: framed serial upload"));
  Serial.println(F("(start/len accept decimal or 0x-prefixed hex)"));
  Serial.println(F("x: print \"Ready.\" for handshaking with serial uploader"));
  Serial.println(F("-----------------------"));
  Serial.println(F("w<ssid><CR>: set STA Wi-Fi SSID"));
  Serial.println(F("p<password><CR>: set STA Wi-Fi password"));
  Serial.println(F("-----------------------"));
}  

// Reads one serial byte with timeout to support robust framed transfers.
bool readSerialByteWithTimeout(uint8_t &outByte, uint32_t timeoutMs) {
  const uint32_t startMs = millis();
  while ((millis() - startMs) < timeoutMs) {
    if (Serial.available() > 0) {
      outByte = static_cast<uint8_t>(Serial.read());
      return true;
    }
    delay(0); yield();
  }
  return false;
}

// Reads an ASCII filename terminated by CR for the next serial upload.
bool readSerialFilename(String &filenameOut) {
  String rawName;
  while (true) {
    uint8_t nextByte = 0;
    if (!readSerialByteWithTimeout(nextByte, kSerialUploadTimeoutMs)) {
      return false;
    }
    if (nextByte == '\r') {
      filenameOut = sanitizeUploadFilename(rawName);
      return true;
    }
    if (nextByte == '\n') {
      continue;
    }
    rawName += static_cast<char>(nextByte);
    if (rawName.length() > kMaxFsPathLength * 2) {
      rawName.remove(kMaxFsPathLength * 2);
    }
  }
}

// Reads one ASCII line terminated by CR from serial input.
bool readSerialLine(String &lineOut, uint32_t timeoutMs) {
  String line;
  while (true) {
    uint8_t nextByte = 0;
    if (!readSerialByteWithTimeout(nextByte, timeoutMs)) {
      return false;
    }
    if (nextByte == '\r') {
      lineOut = line;
      return true;
    }
    if (nextByte == '\n') {
      continue;
    }
    line += static_cast<char>(nextByte);
    if (line.length() > 128) {
      line.remove(128);
    }
  }
}

// Parses decimal or 0x-prefixed hexadecimal unsigned values.
bool parseUnsignedValue(const String &text, uint32_t &valueOut) {
  String token = text;
  token.trim();
  if (token.length() == 0) {
    return false;
  }
  char *endPtr = nullptr;
  const unsigned long parsed = std::strtoul(token.c_str(), &endPtr, 0);
  if (endPtr == token.c_str() || *endPtr != '\0') {
    return false;
  }
  valueOut = static_cast<uint32_t>(parsed);
  return true;
}

// Parses serial dump command format: filename,start,len
bool parseDumpCommandArgs(const String &line, String &filenameOut, uint32_t &startAddrOut,
                          uint16_t &lenOut) {
  String argLine = line;
  argLine.trim();
  const int comma1 = argLine.indexOf(',');
  const int comma2 = (comma1 >= 0) ? argLine.indexOf(',', comma1 + 1) : -1;
  if (comma1 <= 0 || comma2 <= comma1 + 1) {
    return false;
  }
  const String filenameRaw = argLine.substring(0, comma1);
  const String startRaw = argLine.substring(comma1 + 1, comma2);
  const String lenRaw = argLine.substring(comma2 + 1);

  uint32_t parsedStart = 0;
  uint32_t parsedLen = 0;
  if (!parseUnsignedValue(startRaw, parsedStart) || !parseUnsignedValue(lenRaw, parsedLen)) {
    return false;
  }
  if (parsedLen == 0 || parsedLen > 0xFFFFUL) {
    return false;
  }
  String cleanName = sanitizeUploadFilename(filenameRaw);
  if (cleanName.length() == 0) {
    return false;
  }
  filenameOut = cleanName;
  startAddrOut = parsedStart;
  lenOut = static_cast<uint16_t>(parsedLen);
  return true;
}

                                                    
// ##############################################################################
//
//     #     # ######  #       #######    #    ######  
//     #     # #     # #       #     #   # #   #     # 
//     #     # #     # #       #     #  #   #  #     # 
//     #     # ######  #       #     # #     # #     # 
//     #     # #       #       #     # ####### #     # 
//     #     # #       #       #     # #     # #     # 
//      #####  #       ####### ####### #     # ######  
//                                                                                    
// ##############################################################################
// Handles serial upload command.
// ##############################################################################


// Usage: 'u' + lenLo,lenHi + payload + cksLo,cksHi; ACK after len, each 128B, checksum.
void processSerialUploadCommand() {
  if (!fsMounted) {
    Serial.println(F("ERROR: filesystem not mounted."));
    return;
  }

  uint8_t lenLo = 0;
  uint8_t lenHi = 0;
  if (!readSerialByteWithTimeout(lenLo, kSerialUploadTimeoutMs) ||
      !readSerialByteWithTimeout(lenHi, kSerialUploadTimeoutMs)) {
    Serial.write(kSerialNakByte);
    Serial.println(F("ERROR: timeout while reading length word."));
    return;
  }

  const uint16_t payloadLength = static_cast<uint16_t>(lenLo) |
                                 (static_cast<uint16_t>(lenHi) << 8);
  if (payloadLength == 0) {
    Serial.println(F("ERROR: payload length is zero."));
    return;
  }

  if (payloadLength > maxBytesToTransfer) {
    Serial.print(F("ERROR: payload exceeds limit "));
    Serial.println(maxBytesToTransfer);
    return;
  }

  // ACK command and length word, signaling sender to start payload transmission.
  Serial.write(kSerialAckByte);

  if (uploadStagingFile) {
    uploadStagingFile.close();
  }
  currentUploadFsError = false;
  uploadInProgress = true;
  setUploadLed(true);
  const String requestedFilename = pendingSerialFilename;
  pendingSerialFilename = String();
  if (requestedFilename.length() > 0) {
    currentFilePath = makeWebUploadFilePath(requestedFilename);
    lastFilename = requestedFilename;
  } else {
    currentFilePath = makeWebUploadFilePath(F("serial_upload.bin"));
    lastFilename = baseNameFromPath(currentFilePath);
  }
  DPRINT(F("Serial upload path: "));
  DPRINTLN(currentFilePath);
  stagedFileBytes = 0;
  streamOffset = 0;
  clearDataBus();

  LittleFS.remove(currentFilePath);
  uploadStagingFile = LittleFS.open(currentFilePath, "w");
  if (!uploadStagingFile) {
    currentUploadFsError = true;
    uploadInProgress = false;
    setUploadLed(false);
    Serial.println(F("ERROR: failed to open staging file."));
    return;
  }

  uint16_t calculatedChecksum = 0;

  for (uint16_t i = 0; i < payloadLength; ++i) {
    uint8_t dataByte = 0;
    if (!readSerialByteWithTimeout(dataByte, kSerialUploadTimeoutMs)) {
      uploadStagingFile.close();
      LittleFS.remove(currentFilePath);
      stagedFileBytes = 0;
      lastFileBytes = 0;
      currentUploadFsError = true;
      uploadInProgress = false;
      setUploadLed(false);
      Serial.write(kSerialNakByte);
      Serial.println(F("ERROR: timeout waiting for data byte."));
      return;
    }

    const size_t written = uploadStagingFile.write(&dataByte, 1);
    if (written != 1) {
      uploadStagingFile.close();
      LittleFS.remove(currentFilePath);
      stagedFileBytes = 0;
      lastFileBytes = 0;
      currentUploadFsError = true;
      uploadInProgress = false;
      setUploadLed(false);
      Serial.println(F("ERROR: write failed."));
      return;
    }

    ++stagedFileBytes;
    calculatedChecksum = static_cast<uint16_t>(calculatedChecksum + dataByte);

    const uint16_t receivedBytes = static_cast<uint16_t>(i + 1);
    if ((receivedBytes % kSerialAckChunkBytes) == 0 || receivedBytes == payloadLength) {
      Serial.write(kSerialAckByte);
    }

    if ((i & 0x1F) == 0) {
      delay(0); yield();
    }
  }

  uint8_t cksLo = 0;
  uint8_t cksHi = 0;
  if (!readSerialByteWithTimeout(cksLo, kSerialUploadTimeoutMs) ||
      !readSerialByteWithTimeout(cksHi, kSerialUploadTimeoutMs)) {
    uploadStagingFile.close();
    LittleFS.remove(currentFilePath);
    stagedFileBytes = 0;
    lastFileBytes = 0;
    currentUploadFsError = true;
    uploadInProgress = false;
    setUploadLed(false);
    Serial.write(kSerialNakByte);
    Serial.println(F("ERROR: timeout while reading checksum word."));
    return;
  }

  const uint16_t receivedChecksum = static_cast<uint16_t>(cksLo) |
                                    (static_cast<uint16_t>(cksHi) << 8);

  if (receivedChecksum != calculatedChecksum) {
    uploadStagingFile.close();
    LittleFS.remove(currentFilePath);
    stagedFileBytes = 0;
    lastFileBytes = 0;
    currentUploadFsError = true;
    uploadInProgress = false;
    setUploadLed(false);
    Serial.write(kSerialNakByte);
    Serial.print(F("ERROR: checksum mismatch, expected="));
    Serial.print(calculatedChecksum);
    Serial.print(F(", received="));
    Serial.println(receivedChecksum);
    return;
  }

  // ACK checksum receipt only when checksum is valid.
  Serial.write(kSerialAckByte);

  uploadStagingFile.close();
  lastFileBytes = stagedFileBytes;
  uploadInProgress = false;
  setUploadLed(false);
  const uint32_t startAddr = resolveStartAddressForPath(currentFilePath);
  if (!saveStartAddressForFile(currentFilePath, startAddr)) {
    Serial.println(F("ERROR: failed to save start address metadata."));
  }
  lastFilename = baseNameFromPath(currentFilePath);
  lastUploadStartAddr = startAddr;
  if (!saveGlobalSettings()) {
    Serial.println(F("ERROR: failed to save global settings."));
  }
  if (!startPlaybackFromStaging(startAddr)) {
    currentUploadFsError = true;
    Serial.println(F("ERROR: failed to send staged file."));
    return;
  }

  Serial.print(F("Serial upload OK: bytes="));
  Serial.println(stagedFileBytes);
  Serial.println(F("Ready."));
}

// ##############################################################################
//
//     ######  ######  #######  #####  #######  #####   #####  
//     #     # #     # #     # #     # #       #     # #     # 
//     #     # #     # #     # #       #       #       #       
//     ######  ######  #     # #       #####    #####   #####  
//     #       #   #   #     # #       #             #       # 
//     #       #    #  #     # #     # #       #     # #     # 
//     #       #     # #######  #####  #######  #####   #####  
//                                                             
// ##############################################################################
// Processes single-character serial commands (r, c, i, x, n, u, and d).
// ##############################################################################

void processSerialCommands() {
  String dumpLine;
  String hexdumpLine;
  String filename;
  String wifiValue;
  static uint32_t startAddr = 0;
  uint16_t len = 0;
  while (Serial.available() > 0) {
    const char cmd = static_cast<char>(Serial.read());
    switch (cmd) {
        #ifdef JTAG_SPARTAN6
          case 'c':
          case 'C':
            // config SPARTAN 6 FPGA via JTAG (only available in JTAG_SPARTAN6 mode)
            Serial.write(' ');
            #ifdef USE_DY1_DISPLAY
              set_static_message(F("cfg"));
            #endif
            jtagConfigure(currentFilePath);
            printIDcode();
            set_rdy_message();
            break;
      #endif
      case 'e':
      case 'E':
        Serial.write(' ');
        eraseEPROM();
        break;
      case 'r':
      case 'R':
        Serial.write(' ');
        startPlaybackFromStaging(resolveStartAddressForPath(currentFilePath));
        break;
      case 'i':
      case 'I':
        printFileInfo();
        break;
      case 'd':
      case 'D':
        Serial.print(F(" Dump from addr: "));
        if (!readSerialLine(hexdumpLine, kSerialUploadTimeoutMs * 25U)) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: timeout while reading hexdump start address."));
          break;
        }
        hexdumpLine.trim();
        if (hexdumpLine.length() > 0 && !parseUnsignedValue(hexdumpLine, startAddr)) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: invalid start address. Use decimal or 0x-prefixed hex."));
          break;
        }

        hexdumpInputBytes256(startAddr);
        startAddr += 256;
        break;
      case 'l':
      case 'L':
        listLittleFsEntries();
        break;
      case 'h':
      case 'H':
        #ifdef JTAG_SPARTAN6
          printIDcode();
        #endif
        printWebInfo();
        printSerialCommandsInfo();
        break;
      case 'x':
      case 'X':
        Serial.println();
        set_rdy_message();
        Serial.println(F("Ready."));
        break;
      case 't':
      case 'T':
        Serial.write(' ');
        testSPItransfer();
        #ifdef USE_DY1_DISPLAY
          test_display();
        #endif
        set_rdy_message();
        Serial.println(F("Ready."));
       break;
      case 'n':
      case 'N':
        if (!readSerialFilename(filename)) {
          pendingSerialFilename = String();
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: timeout while reading filename."));
        } else {
          pendingSerialFilename = filename;
          Serial.write(kSerialAckByte);
          DPRINT(F("Serial filename accepted: "));
          DPRINTLN(pendingSerialFilename);
        }
        break;
      case 'w':
      case 'W':
        if (!readSerialLine(wifiValue, kSerialUploadTimeoutMs * 25U)) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: timeout while reading SSID."));
          continue;
        }
        wifiValue.trim();
        if (wifiValue.length() == 0) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: SSID must not be empty."));
          continue;
        }
        staSsid = wifiValue;
        if (!saveGlobalSettings()) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: failed to save Wi-Fi SSID."));
          continue;
        }
        Serial.write(kSerialAckByte);
        Serial.print(F("Wi-Fi SSID saved: "));
        Serial.println(staSsid);
        break;
      case 'p':
      case 'P':
        if (!readSerialLine(wifiValue, kSerialUploadTimeoutMs * 25U)) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: timeout while reading password."));
          continue;
        }
        staPassword = wifiValue;
        if (!saveGlobalSettings()) {
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: failed to save Wi-Fi password."));
          continue;
        }
        Serial.write(kSerialAckByte);
        Serial.println(F("Wi-Fi password saved."));
        break;
      case 'u':
      case 'U':
        processSerialUploadCommand();
        break;
      case 'j':
      case 'J':
        #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
          if (!readSerialLine(dumpLine, kSerialUploadTimeoutMs * 25U)) {
            Serial.write(kSerialNakByte);
            Serial.println(F("ERROR: timeout while reading dump arguments."));
            continue;
          }

          if (!parseDumpCommandArgs(dumpLine, filename, startAddr, len)) {
            Serial.write(kSerialNakByte);
            Serial.println(F("ERROR: invalid dump syntax. Use d<filename,start,len><CR>"));
            continue;
          }

          if (uploadInProgress) {
            Serial.write(kSerialNakByte);
            Serial.println(F("ERROR: dump blocked while upload is active."));
            continue;
          }

          if (!writeEPROMtoFile(filename, startAddr, len)) {
            Serial.write(kSerialNakByte);
            Serial.println(F("ERROR: EPROM dump failed."));
            continue;
          }

          lastFilename = baseNameFromPath(filename);
          lastFileBytes = len;
          Serial.write(kSerialAckByte);
          Serial.print(F("Dump OK: file="));
          Serial.print(filename);
          Serial.print(F(", start="));
          Serial.print(startAddr);
          Serial.print(F(", len="));
          Serial.println(len);
        #else
          Serial.write(kSerialNakByte);
          Serial.println(F("ERROR: command d is only available in GODIL mode."));
        #endif
        break;
      default:
        Serial.print(F("ERROR: unknown serial command: "));
        Serial.println(cmd);
        printSerialCommandsInfo();
        break;
    }
  }
}
