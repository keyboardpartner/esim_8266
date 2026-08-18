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
  Serial.print(F("\rReplay...  "));
  if (!fsMounted || stagedFileBytes == 0 || filePath.length() == 0 || !LittleFS.exists(filePath)) {
    Serial.println(F("No file to send."));
    return false;
  }

  if (isNonStreamableFilePath(filePath)) {
    Serial.println(F("Skipping playback: .css/.html files are not streamable."));
    return false;
  }
  #ifdef JTAG_SPARTAN6
    if (isBitstreamFilePath(filePath)) {
      dy1message(F("cfg"));
      drawStringBox("FPGA Cfg", filePath, 0);
      jtagConfigure(filePath);
      printIDcode();
      currentChipTypeIndex = kInvalidChipTypeIndex; // reset to invalid index, meaning no chip type selected
      eraseEPROMsilent(); // erase the EEPROM after configuring the FPGA
      outputChipType(kInvalidChipTypeIndex); // output invalid chip type
      readyMessage();
      return true;
    }
  #endif
  lastStreamedStartAddr = startAddr;

  File playbackFile = LittleFS.open(filePath, "r");
  if (!playbackFile) {
    Serial.println(F("Failed to open playback file."));
    dy1message(F("Fnf"));
    drawMsgBox(F("Error"), F("File not found"), DB_ERROR, 2000);
    return false;
  }
  size_t playbackBytesSent = 0;

  Serial.print(F("File: "));
  Serial.print(filePath);
  Serial.print(F(", startAddr=0x"));
  Serial.print(startAddr, HEX);

  #if defined(DEBUG)
    const uint32_t playbackStartMs = millis();
    DPRINT(F(", bytes="));
    DPRINTLN(stagedFileBytes);
  #endif

  streamOffset = 0;
  setUpSendLed(true);
  dy1message(F("rpl"));
  drawStringBox(filePath, "Sent to 0x" + String(startAddr, HEX), 0);

  outputChipType(currentChipTypeIndex);
  
  startBlockTransfer(startAddr);
  while (playbackFile.available()) {
    const int nextByte = playbackFile.read();
    if (nextByte < 0) {
      break;
    }

    outputByte(static_cast<uint8_t>(nextByte));
    ++streamOffset;
    ++playbackBytesSent;
    ++totalBytesSent;

    if ((streamOffset & 0x1F) == 0) {
      delay(0); yield();
    }
  }
  stopBlockTransfer();

  playbackFile.close();
  Serial.println(F(" ...Done."));
#if defined(DEBUG)
  const uint32_t elapsedMs = millis() - playbackStartMs;
  DPRINT(F("Playback done: file="));
  DPRINT(filePath);
  DPRINT(F(", bytes="));
  DPRINT(playbackBytesSent);
  DPRINT(F(", elapsed_ms="));
  DPRINT(elapsedMs);
  DPRINT(F(", rate_Bps="));
  if (elapsedMs > 0) {
    DPRINTLN(static_cast<unsigned long>((static_cast<uint32_t>(playbackBytesSent) * 1000UL) / elapsedMs));
  } else {
    DPRINTLN(F("n/a"));
  }
#endif
  if (playbackBytesSent < 16384) {
      delay(1000);
  }
  readyMessage();
  setUpSendLed(false);
  clearDataBus();
  return true;
}

