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
bool startPlaybackFromStaging(uint32_t startAddr) {
  if (!fsMounted || stagedFileBytes == 0 || currentFilePath.length() == 0 || !LittleFS.exists(currentFilePath)) {
    Serial.println(F("No file to send."));
    return false;
  }

  if (isNonStreamableFilePath(currentFilePath)) {
    Serial.println(F("Skipping playback: .css/.html files are not streamable."));
    return false;
  }

  #ifdef JTAG_SPARTAN6
    if (isBitstreamFilePath(currentFilePath)) {
      #ifdef USE_DY1_DISPLAY
        set_static_message(F("cfg"));
      #endif
      jtagConfigure(currentFilePath);
      getFPGAversion();
      return true;
    }
  #endif

  File playbackFile = LittleFS.open(currentFilePath, "r");
  if (!playbackFile) {
    Serial.println(F("Failed to open playback file."));
    #ifdef USE_DY1_DISPLAY
      set_static_message(F("Fnf"));
      delay(1000);
    #endif
    return false;
  }
  size_t playbackBytesSent = 0;
#if defined(DEBUG)
  const uint32_t playbackStartMs = millis();
  DPRINT(F("Playback start: file="));
  DPRINT(currentFilePath);
  DPRINT(F(", addr="));
  DPRINT(startAddr);
  DPRINT(F(" (0x"));
  Serial.print(startAddr, HEX);
  DPRINT(F("), bytes="));
  DPRINTLN(stagedFileBytes);
#endif

  streamOffset = 0;
  setUpSendLed(true);
  #ifdef USE_DY1_DISPLAY
    set_static_message(F("rpl"));
  #endif
  
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
  Serial.println(F("Finished playback."));
#if defined(DEBUG)
  const uint32_t elapsedMs = millis() - playbackStartMs;
  DPRINT(F("Playback done: file="));
  DPRINT(currentFilePath);
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
  #ifdef USE_DY1_DISPLAY
    if (playbackBytesSent < 16384) {
      delay(500);
    }
    set_rdy_message();
  #endif
  setUpSendLed(false);
  clearDataBus();
  return true;
}

