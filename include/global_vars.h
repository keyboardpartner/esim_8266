#ifndef GLOBALVARS_H
#define GLOBALVARS_H

#include <Arduino.h>

// Uncomment or define STA_MODE to connect to an existing Wi-Fi network instead of starting an access point.
#define STA_MODE

// uncomment to use Web Server
#define USE_WEB_SERVER

// uncomment to use  DY1 Display
#define USE_DY1_DISPLAY

// Uncomment to enable verbose serial debug output.
// #define DEBUG

// Only one of the following three SPI modes can be enabled at a time. The others must be commented out.

// uncomment to use ESIM EPROM simulator
// #define ESIM_SPI

// uncomment to use GODIL SPI Dual Port mode for RAM emulation
// #define GODIL_SPI

// uncomment to use old PEPS EPROM simulator from Hans Lotter, Conitec
// #define PEPS_SPI

// uncomment to use MOJO board with Spartan XC6SLX9 
#define JTAG_SPARTAN6

constexpr const char *versionString = "0.91";

#ifdef DEBUG
  #define DPRINT(...)    Serial.print(__VA_ARGS__)
  //OR, #define DPRINT(args...)    Serial.print(args)
  #define DPRINTLN(...)  Serial.println(__VA_ARGS__)
  #define DPRINTF(...)    Serial.print(F(__VA_ARGS__))
  #define DPRINTLNF(...) Serial.println(F(__VA_ARGS__)) //printing text using the F macro
#else
  #define DPRINT(...)     //blank line
  #define DPRINTLN(...)   //blank line
  #define DPRINTF(...)    //blank line
  #define DPRINTLNF(...)  //blank line
#endif


void printDivLine() {
  Serial.println(F("----------------------------------------"));
}

// print centered text string with optional padding char to serial
void printCenteredSerial(const String &text, char padChar = '-') {
  const int totalWidth = 40;
  const int textLength = text.length();
  if (textLength >= totalWidth) {
    Serial.println(text);
    return;
  }
  const int padding = (totalWidth - textLength) / 2;
  String line;
  line.reserve(totalWidth);
  for (int i = 0; i < padding; ++i) {
    line += padChar;
  }
  line += text;
  while (line.length() < totalWidth) {
    line += padChar;
  }
  Serial.println(line);
}

#ifdef GODIL_SPI
  #define LATCH_PIN 15 // FPGA SPI /SS
  #define LED_SENDDATA 4
  #define LED_UPLOAD 5
  constexpr const char *kApSsid = "GODIL Uploader";
  constexpr uint32_t maxBytesToTransfer = 32768; // 32KB for GODIL RAM
#endif

#ifdef ESIM_SPI
  #define LATCH_PIN 15 // 74HC595 RCLK (ST_CP) latch pin
  #define STROBE_PIN 5
  #define LED_SENDDATA 4
  #define LED_UPLOAD 2 // LED_BUILTIN on ESP8266 boards
  constexpr const char *kApSsid = "ESIM Uploader";
  constexpr uint32_t maxBytesToTransfer = 65536;
#endif

#ifdef PEPS_SPI
  #define LED_SENDDATA 15
  #define LED_UPLOAD 15
  #define LATCH_CLK 14 // SCLK für 4094
  #define M0_PIN 5
  #define M1_PIN 4
  #define STROBE_PIN 5
  #define DATA_PIN 13
  #define DATA_INVERT // bei invertierendem Bustreiber
  constexpr const char *kApSsid = "PEPS Uploader";
  constexpr uint32_t maxBytesToTransfer = 16384; // 16KB for PEPS RAM
#endif

#ifdef JTAG_SPARTAN6
  #define LATCH_PIN 15 // FPGA SPI /SS
  #include "jtag_send.h"
  constexpr const char *kApSsid = "SPARTAN 6 Uploader";
  constexpr uint32_t maxBytesToTransfer = 65536;
  uint32_t jtagIDcode = 0;
#endif

constexpr const char *kApPassword = "0000";
constexpr const char *kStaSsid = "KeyboardPartner";
constexpr const char *kStaPassword = "my_password";

constexpr uint32_t strobeDelayMicros = 5;
constexpr size_t kMaxFsPathLength = 31;
constexpr uint32_t kSerialUploadTimeoutMs = 200;
constexpr uint8_t kSerialAckByte = 0x06;
constexpr uint8_t kSerialNakByte = 0x15;
constexpr uint16_t kSerialAckChunkBytes = 128;
constexpr const char *kGlobalSettingsPath = "/.settings.ini";
constexpr size_t kMaxGlobalSettingsBytes = 1024;

enum WifiMode {
  WifiMode_Unknown = 0,
  WifiMode_STA,
  WifiMode_AP
};

WifiMode currentWifiMode = WifiMode_Unknown;

#endif  // GLOBALVARS_H