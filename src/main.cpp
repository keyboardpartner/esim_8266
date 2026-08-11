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

ESP8266WebServer server(80);
String wifiModeLabel = "AP";
String wifiIpAddress = "192.168.4.1";

String lastFilename;
size_t lastFileBytes = 0;
size_t totalBytesSent = 0;
bool uploadInProgress = false;
bool fsMounted = false;

File uploadStagingFile;
size_t stagedFileBytes = 0;
size_t streamOffset = 0;
String currentFilePath;
String pendingSerialFilename;
uint32_t webUploadStartAddr = 0;
bool currentUploadStartArgInvalid = false;
uint32_t lastUploadStartAddr = 0;
String staSsid = String(kStaSsid);
String staPassword = String(kStaPassword);

bool currentUploadFsError = false;
String pendingMessage;
bool likelyFreshFsImage = false;

uint32_t fpgaVersion = 0;

String htmlEscape(const String &input);
String urlEncodeComponent(const String &input);
bool startPlaybackFromStaging(uint32_t startAddr = 0);
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
  #ifndef JTAG_SPARTAN6
    pinMode(LED_SENDDATA, OUTPUT);
    digitalWrite(LED_SENDDATA, LOW);
  #endif

  #ifdef JTAG_SPARTAN6
    pinMode(LATCH_PIN, OUTPUT);
    digitalWrite(LATCH_PIN, HIGH);
    Serial.println(F("FPGA Binary Uploader by Carsten Meyer 7/2026"));
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
    Serial.println(F("GODIL Binary Uploader by Carsten Meyer 7/2026"));
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
    Serial.println(F("ESIM Binary Uploader by Carsten Meyer 7/2026"));
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
    Serial.println(F("PEPS Binary Uploader by Carsten Meyer 7/2026"));
    SPI.begin();
    SPI.setFrequency(4000000);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
 #endif

  #ifdef USE_DY1_DISPLAY
    clear_disp(0);
    pinMode(DY1_LATCH_PIN, OUTPUT);
    digitalWrite(DY1_LATCH_PIN, LOW);
    set_static_message(F("rst"));
  #endif

  clearDataBus();
  // Blink LED as a short delay to indicate boot and allow time for the serial monitor to connect.
  #ifdef USE_WEB_SERVER
    for (int i = 0; i < 5; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
  #else
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
  #endif

  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: LittleFS init failed"));
    fsMounted = false;
    #ifdef USE_DY1_DISPLAY
      set_static_message(F("FsE"));
      delay(1000); 
    #endif
   } else {
    fsMounted = true;
    warnIfLikelyFreshFilesystemImage();
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
  #ifdef USE_WEB_SERVER
    #ifdef USE_DY1_DISPLAY
      set_static_message(F("con"));
    #endif
    serverInit();
    #ifdef USE_DY1_DISPLAY
      // display IP address on the 3-digit 7-segment display for a few seconds.
      for (int i = 0; i < 4; ++i) {
        set_number(WiFi.localIP()[i], i == 3 ? -1 : 2); // Display the last octet of the IP address
        delay(350);
      }
      Serial.println(F("Server started."));
      delay(500); // additional delay to make the last digit visible for a bit longer
      set_static_message(wifiModeLabel);
      delay(500); 
      set_static_message(F("on "));
      delay(500);
    #endif
    printWebInfo();
  #else
    #ifdef USE_DY1_DISPLAY
      set_static_message(F("off"));
    #endif
  #endif

  #ifdef GODIL_SPI
    getFPGAversion();
  #endif
  #ifdef JTAG_SPARTAN6
    jtagSetup();
    if (fsMounted) {
      const String startupFpgaPath = F("/fpga_main.bit");
      if (LittleFS.exists(startupFpgaPath)) {
        File startupFpgaFile = LittleFS.open(startupFpgaPath, "r");
        if (startupFpgaFile) {
          startupFpgaFile.close();
           if (isBitstreamFilePath(startupFpgaPath)) {
            #ifdef USE_DY1_DISPLAY
              set_static_message(F("cfg"));
            #endif
            jtagConfigure(startupFpgaPath);
            printIDcode();
            set_rdy_message();
          }
        }
      } else {
        #ifdef USE_DY1_DISPLAY
          set_static_message(F("Err"));
          delay(1000); 
        #endif
        Serial.println(F("File /fpga_main.bit not found, FPGA not configured!"));
      }
    }
  #endif

  listLittleFsEntries();
  printFileInfo();
  printSerialCommandsInfo();
  Serial.println("");
  if (!isBitstreamFilePath(currentFilePath)) {
    startPlaybackFromStaging(resolveStartAddressForPath(currentFilePath));
  }
  Serial.println(F("Ready."));
}

// Main service loop for serial commands, HTTP handling, and playback.
void loop() {
  processSerialCommands();
  #ifdef USE_WEB_SERVER
    server.handleClient();
  #endif
}
