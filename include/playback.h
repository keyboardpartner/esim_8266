#pragma once

// ##############################################################################
//
//     ######  #          #    #     # ######     #     #####  #    # 
//     #     # #         # #    #   #  #     #   # #   #     # #   #  
//     #     # #        #   #    # #   #     #  #   #  #       #  #   
//     ######  #       #     #    #    ######  #     # #       ###    
//     #       #       #######    #    #     # ####### #       #  #   
//     #       #       #     #    #    #     # #     # #     # #   #  
//     #       ####### #     #    #    ######  #     #  #####  #    # 
//                                                                    
// ##############################################################################
// PLAYBACK FILES FROM LITTLEFS
// ##############################################################################

#include <Arduino.h>

// Opens the staged file and sends it in one blocking pass.
bool startPlaybackFromStaging(uint32_t startAddr, const String &filePath) {
  Serial.println();
  if (!fsMounted || stagedFileBytes == 0 || filePath.length() == 0 || !LittleFS.exists(filePath)) {
    Serial.println(F("No file to send."));
    return false;
  }
  if (isNonStreamableFilePath(filePath)) {
    Serial.println(F("Skipping playback: System files are not streamable."));
    return false;
  }
  uint32_t start_time = millis();
  #ifdef JTAG_SPARTAN6
     if (isBitstreamFilePath(filePath)) {
      lastUploadStartAddr = 0;
      lastStreamedStartAddr = 0;
      dy1message(F("cfg"));
      drawStringBox("FPGA Cfg", filePath, 0);
      jtagConfigure(filePath);
      printIDcode();
      currentChipTypeIndex = kInvalidChipTypeIndex; // reset to invalid index, meaning no chip type selected
      // eraseEPROMsilent(); // erase the EEPROM after configuring the FPGA
      outputChipType(kInvalidChipTypeIndex); // output invalid chip type
      return true;
    } else {
      if (currentChipTypeIndex > 10) {
        Serial.println(F("PicoBlaze type, start address set to 0."));
        startAddr = 0;
      }
    }
  #endif
  lastStreamedStartAddr = startAddr;

  uint32_t savedChipType = 0;
  if (loadChipTypeForFile(filePath, savedChipType)) {
    currentChipTypeIndex = savedChipType;
  }

  File playbackFile = LittleFS.open(filePath, "r");
  if (!playbackFile) {
    Serial.println(F("Failed to open playback file."));
    dy1message(F("Fnf"));
    drawMsgBox(F("Error"), F("File not found"), DB_ERROR);
    return false;
  }
  size_t playbackBytesSent = 0;

  Serial.print(F("Replay File: "));
  Serial.print(filePath);
  Serial.print(F(", startAddr=0x"));
  Serial.println(startAddr, HEX);

  streamOffset = 0;
  setUpSendLed(true);
  dy1message(F("rpl"));
  drawStringBox(filePath, "Sent to 0x" + String(startAddr, HEX));

  outputChipType(currentChipTypeIndex);
  
  startBlockTransfer(startAddr);
  uint8_t playbackBuffer[256];
  while (playbackFile.available()) {
    const size_t bytesRead = playbackFile.read(playbackBuffer, sizeof(playbackBuffer));
    if (bytesRead == 0) {
      break;
    }

    for (size_t i = 0; i < bytesRead; ++i) {
      outputByte(playbackBuffer[i]);
      ++streamOffset;
      ++playbackBytesSent;
    }
    delay(0); yield();
  }
  stopBlockTransfer();

  playbackFile.close();
  Serial.print(F("Done in"));
  Serial.printf(" %lu ms, bytes sent: %u\n", (unsigned long)(millis() - start_time), playbackBytesSent);
  clearDataBus();
  return true;
}

