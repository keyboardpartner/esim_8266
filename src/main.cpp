/*
// ############################################################################
//       __ ________  _____  ____  ___   ___  ___
//      / //_/ __/\ \/ / _ )/ __ \/ _ | / _ \/ _ \
//     / ,< / _/   \  / _  / /_/ / __ |/ , _/ // /
//    /_/|_/___/_  /_/____/\____/_/_|_/_/|_/____/
//      / _ \/ _ | / _ \/_  __/ |/ / __/ _ \
//     / ___/ __ |/ , _/ / / /    / _// , _/
//    /_/  /_/ |_/_/|_| /_/ /_/|_/___/_/|_|
//
// ############################################################################
*/

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Created with massive help from ChatGPT 5.3 CODEX

// Upload a binary file to the ESP8266 over Wi-Fi and stream its bytes to a 74HC595 shift register using SPI
// (LSB first, 8-bit parallel output, with a strobe signal to latch the outputs).
// Useful for EPROM simulators loaded by a Centronics-like parallel interface,
// or other applications that require streaming bytes to a shift register.

// 74HC595 SER (DS) to MOSI (GPIO13 / D7), SRCLK (SH_CP) to SCLK (GPIO14 / D5).
// RCLK (ST_CP latch) uses GPIO 5 (D1). Tie /OE low and /SRCLR high for always-enabled output.

// Serial commands: e=erase EPROM, r=replay staged file, c=cancel streaming, i=print info,
// n=next serial upload filename terminated by CR,
// u=lenLo,lenHi,data... upload payload by serial (LE length, timeout-protected),
// d=filename,start,len<CR> dumps EPROM bytes to LittleFS,
// j=hexdump 1024 bytes from inputByte() as 16-byte rows.

// The associated sending app ESIM.EXE uses the following protocol:
// Upload flow in main is now:
// Send u command + 16-bit length word.
// Wait for ACK (1 second timeout).
// Send data in chunks of 128 bytes (last chunk can be smaller).
// After each chunk, wait for ACK (1 second timeout).
// Compute 16-bit checksum (sum of all data bytes modulo 65536).
// Send checksum as 16-bit word LSB first.
// Wait for final ACK (1 second timeout).
// On any timeout or NAK, the app exits with an explicit error message indicating at which stage ACK timed out.
// Added waitForAck(...) with a 1-second timeout per wait.
// Waits for ASCII ACK byte 0x06.
// Returns error on timeout or read failure.
// Still prints incoming text lines and tracks ERROR: messages while waiting.

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <SPI.h>
#include <cstdlib>
#include <cstdio>
#include "global_vars.h" // for #defines
#include "jtag_send.h"

// Display helpers are extracted to a dedicated header.
#include "dy1_display.h"

#ifdef USE_TFT_DISPLAY
  #include <TFT_eSPI.h>
#endif

ESP8266WebServer server(80);


String htmlEscape(const String &input);
String urlEncodeComponent(const String &input);
bool startPlaybackFromStaging(uint32_t startAddr, const String &filePath);
bool parseUnsignedValue(const String &text, uint32_t &valueOut);
bool loadStartAddressForFile(const String &filePath, uint32_t &startAddrOut);
bool loadGlobalSettings();
bool saveGlobalSettings();
bool isNonStreamableFilePath(const String &path);

uint32_t resolveStartAddressForPath(const String &path) {
  uint32_t startAddr = 0;
  if (path.length() > 0) {
    loadStartAddressForFile(path, startAddr);
  }
  return startAddr;
}

// SPI transfer and device I/O helpers are extracted to a dedicated header.
#include "spi_transfer.h"
// SERVER & WEB PAGES
#include "web_server_funcs.h"
// Serial command handlers and parsing helpers are extracted into a dedicated header.
#include "serial_cmd.h"
// Playback staged files
#include "playback.h"

// ##############################################################################
//
//      #####  ####### ####### #     # ######  
//     #     # #          #    #     # #     # 
//     #       #          #    #     # #     # 
//      #####  #####      #    #     # ######  
//           # #          #    #     # #       
//     #     # #          #    #     # #       
//      #####  #######    #     #####  #       
//                                             
// ##############################################################################
// MAIN SETUP AND LOOP
// ##############################################################################

// Initializes serial, SPI, filesystem, Wi-Fi, and web server routes.
void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  
  #ifndef JTAG_SPARTAN6
    pinMode(LED_SENDDATA, OUTPUT);
    digitalWrite(LED_SENDDATA, LOW);
  #endif

  #ifdef JTAG_SPARTAN6
    pinMode(LATCH_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, HIGH);
    Serial.print(F("FPGA Binary Uploader by Carsten Meyer 7/2026 V. "));
    Serial.println(versionString);
    SPI.begin();
    SPI.setFrequency(10000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
  #endif

  #ifdef GODIL_SPI
    pinMode(LED_UPLOAD, OUTPUT);
    digitalWrite(LED_UPLOAD, LOW);
    pinMode(LATCH_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, HIGH);
    Serial.print(F("GODIL Binary Uploader by Carsten Meyer 7/2026 V. "));
    Serial.println(versionString);
    SPI.begin();
    SPI.setFrequency(10000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
  #endif

  #ifdef ESIM_SPI
    pinMode(LATCH_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, HIGH);
    pinMode(STROBE_PIN, OUTPUT);
    digitalWrite(STROBE_PIN, HIGH); 
    Serial.print(F("ESIM Binary Uploader by Carsten Meyer 7/2026 V. "));
    Serial.println(versionString);
    SPI.begin();
    SPI.setFrequency(10000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
  #endif

  #ifdef PEPS_SPI
    pinMode(LATCH_CLK, OUTPUT);
    digitalWrite(LATCH_CLK, LOW);
    pinMode(DATA_PIN, OUTPUT);
    digitalWrite(DATA_PIN, LOW);
    pinMode(M0_PIN, OUTPUT);
    digitalWrite(M0_PIN, HIGH);
    pinMode(M1_PIN, OUTPUT);
    digitalWrite(M1_PIN, HIGH);
    Serial.print(F("PEPS Binary Uploader by Carsten Meyer 7/2026 V. "));
    Serial.println(versionString);
    SPI.begin();
    SPI.setFrequency(4000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
  #endif

  #ifdef USE_DY1_DISPLAY
    clear_disp(0);
    pinMode(DY1_LATCH_PIN, OUTPUT);
    digitalWrite(DY1_LATCH_PIN, LOW);
    dy1message(F("rst"));
  #endif

  #ifdef USE_TFT_DISPLAY
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    tft.init();
    tft.setRotation(3);  //The parameters are: 0, 1, 2, 3, representing the rotation of the screen 0°, 90°, 180°, 270°
    tft.setTextSize(1);
    tft.fillRect(0, 0, 160, 160, TFT_BLACK);
    drawStringBox(F("ESP Uploader"), "Version " + String(versionString));
    delay(500);
  #endif

  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: LittleFS init failed"));
    fsMounted = false;
    dy1message(F("FsE"));
    drawMsgBox(F("Error"), F("LittleFS init failed"), DB_ERROR);
  } else {
    fsMounted = true;
    loadGlobalSettings();
    if (lastFilename.length() > 0) {
      const String restoredPath = normalizeFsPath(lastFilename);
      if (isValidFsPath(restoredPath) && LittleFS.exists(restoredPath)) {
        currentFilePath = restoredPath;
      }
    }
    if (currentFilePath.length() > 0 && LittleFS.exists(currentFilePath)) {
      File restoreFile = LittleFS.open(currentFilePath, "r");
      if (restoreFile) {
        stagedFileBytes = restoreFile.size();
        lastFileBytes = stagedFileBytes;
        lastFilename = baseNameFromPath(currentFilePath);
        restoreFile.close();
      }
    }
  }

  #ifdef GODIL_SPI
    fpgaValid = true;  
    getFPGAversion();
  #endif

  #ifdef JTAG_SPARTAN6
    if (fsMounted) {
      if (LittleFS.exists(startupFpgaPath) && isBitstreamFilePath(startupFpgaPath)) {
        drawStringBox(F("FPGA Config"), startupFpgaPath);
        dy1message(F("cfg"));
        if (jtagCheckIDcode()) {
          jtagConfigure(startupFpgaPath);
          printIDcode();
        }
      } else {
        dy1message(F("Err"));
        drawMsgBox(F("FPGA File"), F("not found!"), DB_ERROR);
        Serial.print(F("File "));
        Serial.print(startupFpgaPath);
        Serial.println(F(" not found, FPGA not configured!"));
      }
    }
  #endif

  if (autoplayEnabled && !isBitstreamFilePath(currentFilePath)) {
    startPlaybackFromStaging(resolveStartAddressForPath(currentFilePath), currentFilePath);
  }

  #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
    outputChipType(currentChipTypeIndex);
    outputPort(0, port0value); // set all outputs
    outputPort(1, port1value); // set all outputs
  #endif
  
  #ifdef USE_WEB_SERVER
    dy1message(F("con"));
    drawMsgBox(F("Wi-Fi"), F("Connect"), DB_INFO);
    serverInit();
    #ifdef USE_DY1_DISPLAY
      // display IP address on the 3-digit 7-segment display for a few seconds.
      if (currentWifiMode == WifiMode_STA) {
        for (int i = 0; i < 4; ++i) {
          dy1number(WiFi.localIP()[i], i == 3 ? -1 : 2); // Display the last octet of the IP address
          delay(350);
        }
      } else {
        for (int i = 0; i < 4; ++i) {
          dy1number(WiFi.softAPIP()[i], i == 3 ? -1 : 2); // Display the last octet of the IP address
          delay(350);
        }
      }
      Serial.println(F("Server started."));
      delay(500); // additional delay to make the last digit visible for a bit longer
      dy1message(wifiModeLabel);
      delay(500); 
      dy1message(F("on "));
      delay(500);
    #endif
    if (currentWifiMode == WifiMode_STA) {
      drawStringBox(F("STA Mode"), WiFi.localIP().toString());
    } else {
      drawStringBox(F("AP Mode"), WiFi.softAPIP().toString());
    }
    printWebInfo();
  #else
    dy1message(F("off"));
    drawMsgBox(F("Wi-Fi"), F("OFF"), DB_INFO);
  #endif
   
  listLittleFsEntries();
  printFileInfo();
  // printSerialCommandsInfo();
  delay(1000);
  readyMessage();
  Serial.println(F("\nReady."));
}

  
// Main service loop for serial commands, HTTP handling, and playback.
void loop() {
  processSerialCommands();
  #ifdef USE_WEB_SERVER
    server.handleClient();
  #endif
  if (millis() > msgTimeout) {
    setUpSendLed(false);
    drawStatusBox();
    msgTimeout = -1; // reset timeout to avoid repeated drawing
  }
}
