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

// Serial commands: r=replay staged file, c=cancel streaming, i=print info,
// n=next serial upload filename terminated by CR,
// u=lenLo,lenHi,data... upload payload by serial (LE length, timeout-protected),
// d=filename,start,len<CR> dumps EPROM bytes to LittleFS.

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


// Uncomment or define STA_MODE to connect to an existing Wi-Fi network instead of starting an access point.
#define STA_MODE
// Uncomment to enable verbose serial debug output.
#define DEBUG

// uncomment to use GODIL SPI Dual Port mode and DY1 Display
#define GODIL_SPI
#define USE_DY1_DISPLAY

// uncomment to use Web Server
#define USE_WEB_SERVER

#if defined(DEBUG)
  #define DBG_PRINT(x) Serial.print(x)
  #define DBG_PRINTLN(x) Serial.println(x)
  #define DBG_PRINTLN0() Serial.println()
#else
  #define DBG_PRINT(x) do {} while (0)
  #define DBG_PRINTLN(x) do {} while (0)
  #define DBG_PRINTLN0() do {} while (0)
#endif

#define LED_SENDDATA 4
#define LATCH_PIN 15 // 74HC595 RCLK (ST_CP) latch pin or FPGA SPI /SS

#ifdef GODIL_SPI
  #define LED_UPLOAD 5
  constexpr const char *kApSsid = "GODIL Uploader";
#else
  #define STROBE_PIN 5
  #define LED_UPLOAD 2 // LED_BUILTIN on ESP8266 boards
  constexpr const char *kApSsid = "ESIM Uploader";
#endif
constexpr const char *kApPassword = "0000";
constexpr const char *kStaSsid = "KeyboardPartner";
constexpr const char *kStaPassword = "z28hev111";

constexpr uint32_t strobeDelayMicros = 5;
constexpr size_t kMaxFsPathLength = 31;
constexpr uint32_t maxBytesToTransfer = 65536;
constexpr uint32_t kSerialUploadTimeoutMs = 200;
constexpr uint8_t kSerialAckByte = 0x06;
constexpr uint8_t kSerialNakByte = 0x15;
constexpr uint16_t kSerialAckChunkBytes = 128;
constexpr const char *kGlobalSettingsPath = "/.settings.ini";
constexpr const char *kLegacyGlobalSettingsPath = "/setting.ini";
constexpr size_t kMaxGlobalSettingsBytes = 1024;

// ################################################################################
// OHO DY1 DISPLAY
// ################################################################################

// Siebensegment-Anzeige 9-stellig, eigene Platine
// Off		-	0
// Bit 0	ist Segment 	e	1	0x01
// Bit 1	ist Segment 	d	2	0x02
// Bit 2	ist Segment 	g	4	0x04
// Bit 3	ist Segment 	a	8	0x08
// Bit 4	ist Segment 	c	16	0x10
// Bit 5	ist Segment 	dpr	32	0x20
// Bit 6	ist Segment 	b	64	0x40
// Bit 7	ist Segment 	f	128	0x80
#define SEGMENT_INVERT 0xFF // 0xFF = Display mit gemeinsamer Anode, 0 = gem.Kathode
#define DIGIT_MAX 2

// Bit-Reihenfolge für SPI.setBitOrder(MSBFIRST) angepasst

const uint8_t letter2segm[96] = {
  // nur "-"
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,

  // #48 =" 0" bis #57 = "9" und "="
  0xDB, 0x0A, 0xF2, 0x7A, 0x2B, 0x79, 0xF9, 0x1A,
  0xFB, 0x7B, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00,

  // @, A..F, H, I, J, L, N, O, P, R, T, U, Y darstellbar

  0xF3, 0xBB, 0xE9, 0xE0, 0xEA, 0xF1, 0xB1, 0xD9,
  0xAB, 0x81, 0x4A, 0x00, 0xC1, 0x9B, 0xA8, 0xE8,
  0xB3, 0x00, 0xA0, 0x79, 0xE1, 0xCB, 0x00, 0x00,
  0x00, 0x6B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t seg_dp = 4;   // DP right
const uint8_t seg_zero = 0xDB;   // Null für Ripple Blanking

#define DY1_LATCH_PIN 16 // Register-Strobe für 74HC595

uint8_t displ_arr[8]; // ausreichend für 8 Digits

// ################################################################################

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

String htmlEscape(const String &input);
String urlEncodeComponent(const String &input);
bool startPlaybackFromStaging(uint32_t startAddr = 0);
bool parseUnsignedValue(const String &text, uint32_t &valueOut);
bool loadStartAddressForFile(const String &filePath, uint32_t &startAddrOut);
bool loadGlobalSettings();
bool saveGlobalSettings();

uint32_t resolveStartAddressForPath(const String &path) {
  uint32_t startAddr = 0;
  if (path.length() > 0) {
    loadStartAddressForFile(path, startAddr);
  }
  return startAddr;
}

#ifdef USE_DY1_DISPLAY

void spi_send_displ_arr() {
  // Bei OHO-Display ist erstes Schieberegister rechte Stelle, in Dreier-Gruppen
  int8_t num_modules = DIGIT_MAX / 3;
  for (int8_t module = num_modules; module >= 0; module--) {
    int8_t start_idx = module * 3;
    for (int8_t idx = start_idx; idx < start_idx + 3; idx++)  {
      SPI.transfer(displ_arr[idx] ^ SEGMENT_INVERT); // ggf invertiert, Common Anode!
    }
  }
  digitalWrite(DY1_LATCH_PIN, HIGH);
  digitalWrite(DY1_LATCH_PIN, LOW);
}


void clear_disp(uint8_t start_pos) {
  for (uint8_t i = start_pos; i <= DIGIT_MAX; i++) {
    displ_arr[i] = 0x00;
  }
  spi_send_displ_arr();
}


void set_letter(uint8_t pos, unsigned char letter) {
// normale Anzeige, alle VFDs, <letter> als ASCII
  if (letter >= 96) letter = letter - 32; // keine Kleinbuchstaben
  uint8_t segments = letter2segm[letter - 32];
  displ_arr[pos] = segments;
}

void set_dp(uint8_t pos) {
  // Dezimalpunkt an dieser Position einschalten
  displ_arr[pos] = displ_arr[pos] | seg_dp;
}


void set_number(int32_t number, int dp_pos, bool ripple_blank = true) {
  // dp_pos = -1: keine Dezimalstelle, 0..DIGIT_MAX: Dezimalpunkt an dieser Position
  // ripple_blank = true: führende Nullen werden als Leerzeichen angezeigt
  // ripple_blank = false: führende Nullen werden als "0" angezeigt
  // Ein- oder mehrstellige Zahl <number> anzeigen
  int8_t idx;
  set_letter(DIGIT_MAX, (number % 10) + 48); // Einerstelle immer anzeigen, auch wenn 0
  number = number / 10;
  for (idx = DIGIT_MAX - 1; idx >= 0; idx--) {
    set_letter(idx, (number % 10) + 48);
    number = number / 10;
  }
  if (ripple_blank) {
    for (idx = 0; idx <= DIGIT_MAX; idx++) {
      if (displ_arr[idx] == seg_zero) {   // Segment-Null?
        set_letter(idx, ' '); // Leerzeichen
      } else {
        break;
      }
    }
  }
  if (dp_pos >= 0 && dp_pos <= DIGIT_MAX) {
    set_dp(dp_pos);
  }
  spi_send_displ_arr();
}


void set_static_message(String msg) {
  for (uint8_t i = 0; (i < msg.length()) && (i <= DIGIT_MAX); i++) {
    set_letter(i, msg.charAt(i));
  }
  spi_send_displ_arr();
}

void test_display() {
  Serial.println(F("Testing OHO display..."));
  uint8_t seg = 1;
  for (uint8_t count = 0; count <= 7; count++) {
    for (uint8_t pos = 0; pos <= DIGIT_MAX; pos++) {
      displ_arr[pos] = seg;
    }
    seg = seg << 1;
    spi_send_displ_arr();
    delay(100);
  }
  set_number(123, 2);
  Serial.println(F("Ready."));
}

#endif

// ################################################################################
//
//     ######  ########  ####    ##     ## ######## ######## ########  
//    ##    ## ##     ##  ##      ##   ##  ##       ##       ##     ## 
//    ##       ##     ##  ##       ## ##   ##       ##       ##     ## 
//     ######  ########   ##        ###    ######   ######   ########  
//          ## ##         ##       ## ##   ##       ##       ##   ##   
//    ##    ## ##         ##      ##   ##  ##       ##       ##    ##  
//     ######  ##        ####    ##     ## ##       ######## ##     ## 
//
// ################################################################################

// SPI Transfer functions

// Controls the onboard LED used as HTTP upload activity indicator.
void setUploadLed(bool on) {
  digitalWrite(LED_UPLOAD, on ? HIGH : LOW);
}

// Controls a dedicated LED that indicates active file playback/streaming.
void setUpSendLed(bool on) {
  digitalWrite(LED_SENDDATA, on ? HIGH : LOW);
}


#ifdef GODIL_SPI

// Clears all output bits on the shift register.
void clearDataBus() {

}

uint32_t spi_xfer32_ss(uint32_t data) {
  uint32_t rxlong;
  digitalWrite(LATCH_PIN, LOW);
  rxlong  = SPI.transfer16(data >> 16) << 16;
  rxlong |= SPI.transfer16(data & 0xFFFF);
  digitalWrite(LATCH_PIN, HIGH);
  return rxlong;
}

// Sends one byte to device output
void outputByte(uint8_t value, uint32_t addr) {
  uint32_t txlong = (addr << 8) | value;
  // Shifts one byte to the 74HC595 using SPI and latches the new output state.
  digitalWrite(LATCH_PIN, LOW);
  SPI.write32(txlong | 0x80000000); // write command and address to write
  digitalWrite(LATCH_PIN, HIGH);
}

// receives one byte from device
uint8_t inputByte(uint32_t addr) {
  uint32_t txlong = (addr << 8);
  digitalWrite(LATCH_PIN, LOW);
  SPI.write32(txlong); // set address to read from internal register
  digitalWrite(LATCH_PIN, HIGH);
  uint32_t rxlong = spi_xfer32_ss(0) & 0xFF; // read back data from internal register
  return static_cast<uint8_t>(rxlong);
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

  LittleFS.remove(filename);
  File outFile = LittleFS.open(filename, "w");
  if (!outFile) {
    Serial.print(F("ERROR: cannot open file for writing: "));
    Serial.println(filename);
    return false;
  }

  for (uint32_t i = 0; i < len; ++i) {
    const uint8_t value = inputByte(start_addr + i);
    if (outFile.write(&value, 1) != 1) {
      outFile.close();
      LittleFS.remove(filename);
      Serial.print(F("ERROR: write failed at offset "));
      Serial.println(i);
      return false;
    }

    if ((i & 0x1F) == 0) {
      delay(0);
    }
  }

  outFile.close();
  Serial.print(F("EPROM dump saved to "));
  Serial.print(filename);
  Serial.print(F(", bytes="));
  Serial.println(len);
  return true;
}

void testSPItransfer() {
  // Write test pattern to internal registers and read back to verify correctness.
  Serial.println(F("Testing GODIL SPI transfer..."));
  for (uint32_t i = 0; i < 0x0400; ++i) {
    outputByte(1 << (i % 8), i); // DIL Tester LED Test pattern
  }
  uint32_t start_addr = 0x0400;
  uint32_t multiplier = 19;
  uint8_t vals_written[16];
  for (uint32_t i = 0; i < 16; ++i) {
    vals_written[i] = static_cast<uint8_t>(0xFF - i*11);
    outputByte(vals_written[i], start_addr + i*multiplier);
  }
  for (uint32_t i = 0; i < 16; ++i) {
    // read back data from internal register
    Serial.print(F("Addr 0x"));
    Serial.print(start_addr + i*multiplier, HEX);
    Serial.print(F(", Written 0x"));
    Serial.print(vals_written[i], HEX);
    Serial.print(F(", Received 0x"));
    Serial.println(inputByte(start_addr + i*multiplier), HEX);
  }
}

#else

// Clears all output bits on the shift register.
void clearDataBus() {
  digitalWrite(LATCH_PIN, LOW);
  SPI.transfer(0);
  digitalWrite(LATCH_PIN, HIGH);
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
void outputByte(uint8_t value, uint32_t addr) {
  // Shifts one byte to the 74HC595 using SPI and latches the new output state.
  digitalWrite(LATCH_PIN, LOW);
  SPI.transfer(value);
  digitalWrite(LATCH_PIN, HIGH);
  pulseStrobe();
  delayMicroseconds(strobeDelayMicros);
}

void testSPItransfer() {
  Serial.println(F("Testing SPI transfer not supported in this mode."));
}

#endif



// ##############################################################################
//
//     ######  ######## ########  ##     ## ######## ########
//    ##    ## ##       ##     ## ##     ## ##       ##     ##
//    ##       ##       ##     ## ##     ## ##       ##     ##
//     ######  ######   ########  ##     ## ######   ########
//          ## ##       ##   ##    ##   ##  ##       ##   ##
//    ##    ## ##       ##    ##    ## ##   ##       ##    ##
//     ######  ######## ##     ##    ###    ######## ##     ##  
//                                                     
// ##############################################################################


void redirectToRoot() {
  server.sendHeader("Location", "/", true);
  server.send(303, "text/plain", "");
}

String baseNameFromPath(const String &input) {
  String name = input;
  name.trim();
  name.replace('\\', '/');
  const int lastSlash = name.lastIndexOf('/');
  if (lastSlash >= 0 && lastSlash < static_cast<int>(name.length()) - 1) {
    name = name.substring(lastSlash + 1);
  } else if (lastSlash == static_cast<int>(name.length()) - 1) {
    name = String();
  }
  return name;
}

String sanitizeUploadFilename(const String &input) {
  String name = baseNameFromPath(input);
  String out;
  out.reserve(name.length());

  for (char c : name) {
    const bool isAlphaNum =
        (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    if (isAlphaNum || c == '.' || c == '_' || c == '-') {
      out += c;
    } else {
      out += '_';
    }
  }

  while (out.length() > 0 && out[0] == '.') {
    out.remove(0, 1);
  }

  if (out.length() == 0) {
    out = F("upload.bin");
  }

  return out;
}

String selectWebUploadName(const String &multipartName, const String &formName) {
  String chosen = sanitizeUploadFilename(multipartName);
  const String fallback = sanitizeUploadFilename(formName);

  // Some clients send a placeholder multipart filename; prefer explicit form-provided name.
  if (chosen.equalsIgnoreCase(F("upload.bin")) && fallback.length() > 0 &&
      !fallback.equalsIgnoreCase(F("upload.bin"))) {
    chosen = fallback;
  }

  return chosen;
}

String splitBaseAndExtension(const String &name, String &base, String &ext) {
  const int dot = name.lastIndexOf('.');
  if (dot > 0 && dot < static_cast<int>(name.length()) - 1) {
    base = name.substring(0, dot);
    ext = name.substring(dot);
  } else {
    base = name;
    ext = String();
  }
  return base;
}

String fitFilenameToLength(const String &rawName, size_t maxNameLen) {
  String name = sanitizeUploadFilename(rawName);
  if (name.length() <= maxNameLen) {
    return name;
  }

  String base;
  String ext;
  splitBaseAndExtension(name, base, ext);

  if (ext.length() >= maxNameLen) {
    return name.substring(0, maxNameLen);
  }

  const size_t baseMax = maxNameLen - ext.length();
  if (baseMax == 0) {
    return name.substring(0, maxNameLen);
  }

  return base.substring(0, baseMax) + ext;
}

String addSuffixBeforeExtension(const String &name, const String &suffix, size_t maxNameLen) {
  String base;
  String ext;
  splitBaseAndExtension(name, base, ext);

  if (suffix.length() + ext.length() >= maxNameLen) {
    return fitFilenameToLength(name, maxNameLen);
  }

  const size_t baseMax = maxNameLen - suffix.length() - ext.length();
  if (baseMax == 0) {
    return fitFilenameToLength(name, maxNameLen);
  }

  return base.substring(0, baseMax) + suffix + ext;
}

String makeWebUploadFilePath(const String &displayFilename) {
  const size_t maxNameLen = (kMaxFsPathLength > 1) ? (kMaxFsPathLength - 1) : 1;
  const String fitted = fitFilenameToLength(displayFilename, maxNameLen);

  String path = String('/') + fitted;
  if (!LittleFS.exists(path)) {
    return path;
  }

  for (uint16_t i = 1; i < 1000; ++i) {
    const String suffix = String('_') + String(i);
    const String candidate = addSuffixBeforeExtension(fitted, suffix, maxNameLen);
    path = String('/') + candidate;
    if (!LittleFS.exists(path)) {
      return path;
    }
  }

  // Last-resort fallback when many colliding names exist.
  return String('/') + fitFilenameToLength(String("upload_") + String(millis()) + String(".bin"), maxNameLen);
}

bool isValidFsPath(const String &path) {
  return path.length() > 1 && path[0] == '/' && path.indexOf("..") < 0;
}

String normalizeFsPath(const String &rawPath) {
  String path = rawPath;
  path.trim();
  if (path.length() > 0 && path[0] != '/') {
    path = String('/') + path;
  }
  return path;
}

String makePerFileIniPath(const String &filePath) {
  const String cleanPath = normalizeFsPath(filePath);
  String name = baseNameFromPath(cleanPath);

  if (name.length() == 0) {
    return String();
  }

  const size_t maxNameLen = (kMaxFsPathLength > 1) ? (kMaxFsPathLength - 1) : 1;
  String iniName = name + F(".ini");
  if (iniName.length() > maxNameLen) {
    const size_t nameMax = (maxNameLen > 4) ? (maxNameLen - 4) : 1;
    iniName = name.substring(0, nameMax) + F(".ini");
  }

  return String('/') + iniName;
}

bool saveStartAddressForFile(const String &filePath, uint32_t startAddr) {
  if (!fsMounted) {
    return false;
  }

  const String iniPath = makePerFileIniPath(filePath);
  if (iniPath.length() == 0) {
    return false;
  }

  File iniFile = LittleFS.open(iniPath, "w");
  if (!iniFile) {
    return false;
  }

  iniFile.printf("start=%lu\n", static_cast<unsigned long>(startAddr));
  iniFile.close();
  return true;
}

bool loadStartAddressForFile(const String &filePath, uint32_t &startAddrOut) {
  if (!fsMounted) {
    return false;
  }

  const String iniPath = makePerFileIniPath(filePath);
  if (iniPath.length() == 0 || !LittleFS.exists(iniPath)) {
    return false;
  }

  File iniFile = LittleFS.open(iniPath, "r");
  if (!iniFile) {
    return false;
  }

  bool found = false;
  while (iniFile.available()) {
    String line = iniFile.readStringUntil('\n');
    line.trim();
    if (line.length() == 0 || line.startsWith("#") || line.startsWith(";")) {
      continue;
    }

    const int sep = line.indexOf('=');
    if (sep <= 0) {
      continue;
    }

    const String key = line.substring(0, sep);
    const String value = line.substring(sep + 1);
    if (key == F("start")) {
      uint32_t parsed = 0;
      if (parseUnsignedValue(value, parsed)) {
        startAddrOut = parsed;
        found = true;
      }
      break;
    }
  }

  iniFile.close();
  return found;
}

bool saveGlobalSettings() {
  if (!fsMounted) {
    return false;
  }

  File iniFile = LittleFS.open(kGlobalSettingsPath, "w");
  if (!iniFile) {
    return false;
  }

  iniFile.print(F("last_filename="));
  iniFile.println(lastFilename);
  iniFile.print(F("last_start="));
  iniFile.println(static_cast<unsigned long>(lastUploadStartAddr));
  iniFile.print(F("sta_ssid="));
  iniFile.println(staSsid);
  iniFile.print(F("sta_password="));
  iniFile.println(staPassword);
  iniFile.close();
  return true;
}

bool loadGlobalSettings() {
  if (!fsMounted) {
    return false;
  }

  String settingsPath;
  if (LittleFS.exists(kGlobalSettingsPath)) {
    settingsPath = kGlobalSettingsPath;
  } else if (LittleFS.exists(kLegacyGlobalSettingsPath)) {
    settingsPath = kLegacyGlobalSettingsPath;
  } else {
    return false;
  }

  File iniFile = LittleFS.open(settingsPath, "r");
  if (!iniFile) {
    return false;
  }

  if (iniFile.size() > kMaxGlobalSettingsBytes) {
    iniFile.close();
    Serial.println(F("WARN: global settings file too large, ignoring."));
    return false;
  }

  bool loadedAnything = false;
  while (iniFile.available()) {
    String line = iniFile.readStringUntil('\n');
    line.trim();
    if (line.length() == 0 || line.startsWith("#") || line.startsWith(";")) {
      continue;
    }

    const int sep = line.indexOf('=');
    if (sep <= 0) {
      continue;
    }

    String key = line.substring(0, sep);
    String value = line.substring(sep + 1);
    key.trim();
    value.trim();

    if (key == F("last_filename")) {
      const size_t maxNameLen = (kMaxFsPathLength > 1) ? (kMaxFsPathLength - 1) : 1;
      lastFilename = fitFilenameToLength(sanitizeUploadFilename(value), maxNameLen);
      loadedAnything = true;
    } else if (key == F("last_start")) {
      uint32_t parsed = 0;
      if (parseUnsignedValue(value, parsed)) {
        lastUploadStartAddr = parsed;
        loadedAnything = true;
      }
    } else if (key == F("sta_ssid")) {
      staSsid = value;
      loadedAnything = true;
    } else if (key == F("sta_password")) {
      staPassword = value;
      loadedAnything = true;
    }
  }

  iniFile.close();

  if (staSsid.length() == 0) {
    staSsid = String(kStaSsid);
  }

  // Migrate legacy path to hidden reserved settings path to avoid filename collisions.
  if (settingsPath == kLegacyGlobalSettingsPath) {
    saveGlobalSettings();
  }

  return loadedAnything;
}

void deleteStartAddressIniForFile(const String &filePath) {
  if (!fsMounted) {
    return;
  }

  const String iniPath = makePerFileIniPath(filePath);
  if (iniPath.length() == 0) {
    return;
  }

  if (LittleFS.exists(iniPath)) {
    LittleFS.remove(iniPath);
  }
}

String formatAddressForInput(uint32_t addr) {
  char buf[16];
  std::snprintf(buf, sizeof(buf), "0x%lX", static_cast<unsigned long>(addr));
  return String(buf);
}

String buildFsDirectoryHtml() {
  String html;
  html.reserve(3200);
  html += F("<h2>Files present</h2>");

  if (!fsMounted) {
    html += F("<p>LittleFS is not mounted.</p>");
    return html;
  }

  Dir dir = LittleFS.openDir("/");
  bool any = false;

  html += F("<table style='width:100%;border-collapse:collapse'>");
  html += F("<thead><tr>");
  html += F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Name</th>");
  html += F("<th style='text-align:right;border-bottom:1px solid #334155;padding:6px 8px'>Size</th>");
  html += F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Start (dec/0x)</th>");
  html += F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Actions</th>");
  html += F("</tr></thead><tbody>");

  while (dir.next()) {
    const String path = normalizeFsPath(dir.fileName());
    if (path.endsWith(F(".ini"))) {
      continue;
    }

    any = true;
    const size_t bytes = dir.fileSize();
    const String displayName = baseNameFromPath(path);
    const String displayText = displayName.length() ? displayName : path;

    html += F("<tr>");
    html += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937'><a href='/download-file?path=");
    html += urlEncodeComponent(path);
    html += F("' style='color:#93c5fd;text-decoration:none'><code>");
    html += htmlEscape(displayText);
    html += F("</code></a></td>");
    html += F("<td style='padding:6px 8px;text-align:right;border-bottom:1px solid #1f2937'>");
    html += String(bytes);
    html += F(" bytes</td>");

    uint32_t startAddr = 0;
    loadStartAddressForFile(path, startAddr);
    html += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937'>");
    html += F("<input name='start' type='text' value='");
    html += htmlEscape(formatAddressForInput(startAddr));
    html += F("' required form='stream_");
    html += urlEncodeComponent(path);
    html += F("' style='margin:0;padding:7px 9px;border-radius:8px;max-width:130px'>");
    html += F("</td>");

    html += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937;white-space:nowrap;vertical-align:middle'>");
    html += F("<div class='actions'>");

    html += F("<form id='stream_");
    html += urlEncodeComponent(path);
    html += F("' method='POST' action='/stream-file'>");
    html += F("<input type='hidden' name='path' value='");
    html += htmlEscape(path);
    html += F("'><button class='action-btn' type='submit'>Stream</button></form>");

    html += F("<form method='POST' action='/delete-file' onsubmit=\"return confirm('Delete file?');\">");
    html += F("<input type='hidden' name='path' value='");
    html += htmlEscape(path);
    html += F("'><button class='action-btn' type='submit'>Delete</button></form>");

    html += F("</div></td></tr>");
  }

  if (!any) {
    html += F("<tr><td colspan='4' style='padding:8px;color:#94a3b8'>No files present.</td></tr>");
  }

  html += F("</tbody></table>");
  return html;
}

// Prints current staged file and streaming state for serial debugging.
void printFileInfo() {
#if defined(DEBUG)
  const bool stagedExists = fsMounted && currentFilePath.length() > 0 && LittleFS.exists(currentFilePath);
  size_t stagedSize = 0;

  if (stagedExists) {
    File infoFile = LittleFS.open(currentFilePath, "r");
    if (infoFile) {
      stagedSize = infoFile.size();
      infoFile.close();
    }
  }

  Serial.println(F("File Info"));
  Serial.print(F("fsMounted: "));
  Serial.println(fsMounted ? F("true") : F("false"));
  Serial.print(F("stagedExists: "));
  Serial.println(stagedExists ? F("true") : F("false"));
  Serial.print(F("stagedPath: "));
  Serial.println(currentFilePath.length() ? currentFilePath : String(F("none")));
  Serial.print(F("stagedFileBytes: "));
  Serial.println(stagedFileBytes);
  Serial.print(F("stagedFileSizeOnFs: "));
  Serial.println(stagedSize);
  Serial.print(F("lastFilename: "));
  Serial.println(lastFilename.length() ? lastFilename : String(F("none")));
  Serial.print(F("lastFileBytes: "));
  Serial.println(lastFileBytes);
#else
  Serial.println(F("Serial command 'i': DEBUG is disabled."));
#endif
}

// Lists all LittleFS root directory entries on serial.
void listLittleFsEntries() {
  if (!fsMounted) {
    Serial.println(F("ERROR: filesystem not mounted."));
    return;
  }

  Dir dir = LittleFS.openDir("/");
  size_t entryCount = 0;
  size_t totalBytes = 0;

  Serial.println(F("LittleFS directory entries:"));
  while (dir.next()) {
    const String path = normalizeFsPath(dir.fileName());
    const size_t bytes = dir.fileSize();
    Serial.print(F("  "));
    Serial.print(path);
    Serial.print(F(" ("));
    Serial.print(bytes);
    Serial.println(F(" bytes)"));
    ++entryCount;
    totalBytes += bytes;
    delay(0);
  }

  if (entryCount == 0) {
    Serial.println(F("  <empty>"));
  }

  Serial.print(F("Entries: "));
  Serial.print(entryCount);
  Serial.print(F(", total bytes: "));
  Serial.println(totalBytes);
}

void printWebInfo() {
  Serial.print(F("Wi-Fi mode: "));
  Serial.println(wifiModeLabel);
  Serial.print(F("Wi-Fi STA SSID: "));
  Serial.println(staSsid.length() ? staSsid : String(F("<empty>")));
  Serial.print(F("IP address: "));
  Serial.println(wifiIpAddress);
  Serial.print(F("AP SSID: "));
  Serial.println(kApSsid);
  Serial.print(F("Last file: "));
  Serial.println(lastFilename.length() ? lastFilename : String(F("none")));
  Serial.print(F("Last file size: "));
  Serial.println(lastFileBytes);
  Serial.println(F("Serial commands:"));
  Serial.println(F("  h: print web/status info and help text (this one)"));
  Serial.println(F("  r: replay staged file (blocking)"));
  Serial.println(F("  i: print file debug info"));
  Serial.println(F("  l: list LittleFS directory entries"));
  Serial.println(F("  x: print \"Ready.\" for handshaking with serial uploader"));
  Serial.println(F("  t: test SPI transfer"));
  Serial.println(F("  w<ssid><CR>: set STA Wi-Fi SSID"));
  Serial.println(F("  p<password><CR>: set STA Wi-Fi password"));
  Serial.println(F("  n<filename><CR>: set next serial upload filename"));
  Serial.println(F("  u<lenLo><lenHi><data...><cksLo><cksHi>: framed serial upload"));
  Serial.println(F("  d<filename,start,len><CR>: dump EPROM to file"));
  Serial.println(F("     start/len accept decimal or 0x-prefixed hex"));
}  // namespace


// Reads one serial byte with timeout to support robust framed transfers.
bool readSerialByteWithTimeout(uint8_t &outByte, uint32_t timeoutMs) {
  const uint32_t startMs = millis();
  while ((millis() - startMs) < timeoutMs) {
    if (Serial.available() > 0) {
      outByte = static_cast<uint8_t>(Serial.read());
      return true;
    }
    delay(0);
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

String urlEncodeComponent(const String &input) {
  static const char *kHex = "0123456789ABCDEF";
  String encoded;
  encoded.reserve(input.length() * 3);

  for (char c : input) {
    const bool isAlphaNum =
        (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    const bool isUnreserved = isAlphaNum || c == '-' || c == '_' || c == '.' || c == '~';

    if (isUnreserved) {
      encoded += c;
    } else {
      const uint8_t value = static_cast<uint8_t>(c);
      encoded += '%';
      encoded += kHex[(value >> 4) & 0x0F];
      encoded += kHex[value & 0x0F];
    }
  }

  return encoded;
}


// Escapes user-controlled text before inserting it into HTML output.
String htmlEscape(const String &input) {
  String escaped;
  escaped.reserve(input.length() + 16);

  for (char c : input) {
    switch (c) {
      case '&':
        escaped += F("&amp;");
        break;
      case '<':
        escaped += F("&lt;");
        break;
      case '>':
        escaped += F("&gt;");
        break;
      case '"':
        escaped += F("&quot;");
        break;
      default:
        escaped += c;
        break;
    }
  }

  return escaped;
}

// Streams the current LittleFS directory table to keep heap usage low.
void sendFsDirectoryHtmlStreamed() {
  server.sendContent(F("<h2>Files present</h2>"));

  if (!fsMounted) {
    server.sendContent(F("<p>LittleFS is not mounted.</p>"));
    return;
  }

  Dir dir = LittleFS.openDir("/");
  bool any = false;

  server.sendContent(F("<table style='width:100%;border-collapse:collapse'>"));
  server.sendContent(F("<thead><tr>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Name</th>"));
  server.sendContent(F("<th style='text-align:right;border-bottom:1px solid #334155;padding:6px 8px'>Size</th>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Start (dec/0x)</th>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Actions</th>"));
  server.sendContent(F("</tr></thead><tbody>"));

  while (dir.next()) {
    const String path = normalizeFsPath(dir.fileName());
    if (path.endsWith(F(".ini"))) {
      continue;
    }

    any = true;
    const size_t bytes = dir.fileSize();
    const String displayName = baseNameFromPath(path);
    const String displayText = displayName.length() ? displayName : path;

    uint32_t startAddr = 0;
    loadStartAddressForFile(path, startAddr);

    String row;
    row.reserve(900);
    row += F("<tr>");
    row += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937'><a href='/download-file?path=");
    row += urlEncodeComponent(path);
    row += F("' style='color:#93c5fd;text-decoration:none'><code>");
    row += htmlEscape(displayText);
    row += F("</code></a></td>");
    row += F("<td style='padding:6px 8px;text-align:right;border-bottom:1px solid #1f2937'>");
    row += String(bytes);
    row += F(" bytes</td>");
    row += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937'>");
    row += F("<input name='start' type='text' value='");
    row += htmlEscape(formatAddressForInput(startAddr));
    row += F("' required form='stream_");
    row += urlEncodeComponent(path);
    row += F("' style='margin:0;padding:7px 9px;border-radius:8px;max-width:130px'>");
    row += F("</td>");
    row += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937;white-space:nowrap;vertical-align:middle'>");
    row += F("<div class='actions'>");
    row += F("<form id='stream_");
    row += urlEncodeComponent(path);
    row += F("' method='POST' action='/stream-file'>");
    row += F("<input type='hidden' name='path' value='");
    row += htmlEscape(path);
    row += F("'><button class='action-btn' type='submit'>Stream</button></form>");
    row += F("<form method='POST' action='/delete-file' onsubmit=\"return confirm('Delete file?');\">");
    row += F("<input type='hidden' name='path' value='");
    row += htmlEscape(path);
    row += F("'><button class='action-btn' type='submit'>Delete</button></form>");
    row += F("</div></td></tr>");
    server.sendContent(row);
    delay(0);
  }

  if (!any) {
    server.sendContent(F("<tr><td colspan='4' style='padding:8px;color:#94a3b8'>No files present.</td></tr>"));
  }

  server.sendContent(F("</tbody></table>"));
}

// Streams the uploader page in small chunks to avoid large temporary String allocations.
void handleRoot() {
  const String message = pendingMessage;
  pendingMessage = String();

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  server.sendContent(F("<!doctype html><html><head><meta charset='utf-8'>"));
  server.sendContent(F("<meta name='viewport' content='width=device-width,initial-scale=1'>"));
  server.sendContent(F("<title>ESIM/GODIL Binary Uploader</title>"));
  server.sendContent(F("<style>"));
  server.sendContent(F("body{font-family:Arial,sans-serif;background:#0f172a;color:#e2e8f0;margin:0;padding:24px;}"));
  server.sendContent(F(".card{width:780px;max-width:calc(100vw - 48px);margin:0 auto;background:#111827;border:1px solid #334155;border-radius:16px;padding:24px;box-shadow:0 20px 50px rgba(0,0,0,.25);box-sizing:border-box;}h1{margin-top:0;font-size:28px;}p,li{line-height:1.5;}label{display:block;margin:16px 0 8px;}input[type=file],input[type=text]{display:block;width:100%;padding:12px;background:#0b1220;border:1px solid #334155;color:#e2e8f0;border-radius:10px;box-sizing:border-box;}button{margin-top:16px;background:#22c55e;border:0;color:#052e16;font-weight:700;padding:12px 18px;border-radius:10px;cursor:pointer;}button[disabled]{opacity:.65;cursor:not-allowed;}.actions{display:inline-flex;align-items:center;gap:6px;}.actions form{margin:0;}.action-btn{display:inline-flex;align-items:center;justify-content:center;min-width:84px;height:25px;margin:0;padding:0 10px;font-size:12px;font-weight:700;line-height:1;border-radius:10px;border:0;background:#22c55e;color:#052e16;text-decoration:none;cursor:pointer;box-sizing:border-box;vertical-align:middle;}code{background:#0b1220;padding:2px 6px;border-radius:6px;}.muted{color:#94a3b8;}.messages{min-height:72px;margin:16px 0;display:flex;flex-direction:column;justify-content:flex-start;}.msg{margin:0;padding:12px 14px;background:#0b1220;border-left:4px solid #38bdf8;border-radius:8px;}.msg.wait{border-left-color:#f59e0b;}.msg.ok{border-left-color:#22c55e;}</style></head><body><div class='card'>"));

#ifdef GODIL_SPI
  server.sendContent(F("<h1>GODIL Binary Uploader</h1><h3>by KeyboardPartner 7/2026</h3>"));
#else
  server.sendContent(F("<h1>ESIM Binary Uploader</h1><h3>by KeyboardPartner 7/2026</h3>"));
#endif

  server.sendContent(F("<form id='uploadForm' method='POST' action='/upload' enctype='multipart/form-data'>"));
  server.sendContent(F("<table style='width:100%;border-collapse:separate;border-spacing:8px 6px;margin:0'><tr>"));
  server.sendContent(F("<td style='padding:0;width:78%'><label for='binfile' style='display:block;margin:0 0 6px'>Binary file</label><input id='binfile' name='binfile' type='file' accept='.bin,application/octet-stream' required></td>"));
  server.sendContent(F("<td style='padding:0;width:22%'><label for='uploadStart' style='display:block;margin:0 0 6px'>Start (dec/0x)</label><input id='uploadStart' name='start' type='text' value='"));
  server.sendContent(htmlEscape(formatAddressForInput(lastUploadStartAddr)));
  server.sendContent(F("' maxlength='10' style='max-width:11ch' required></td>"));
  server.sendContent(F("</tr></table>"));
  server.sendContent(F("<input id='uploadName' name='uploadName' type='hidden' value=''>"));
  server.sendContent(F("<button id='uploadButton' type='submit'>Upload and stream</button></form>"));

#ifdef GODIL_SPI
  server.sendContent(F("<h3>Dump EPROM</h3>"));
  server.sendContent(F("<form method='POST' action='/dump-eprom'>"));
  server.sendContent(F("<table style='width:100%;border-collapse:separate;border-spacing:8px 6px;margin:0'><tr>"));
  server.sendContent(F("<td style='padding:0;width:50%'><label for='dumpFilename' style='display:block;margin:0 0 6px'>Filename</label><input id='dumpFilename' name='filename' type='text' value='eprom_dump.bin' required></td>"));
  server.sendContent(F("<td style='padding:0;width:28%'><label for='dumpStart' style='display:block;margin:0 0 6px'>Start (dec/0x)</label><input id='dumpStart' name='start' type='text' value='0x0000' required></td>"));
  server.sendContent(F("<td style='padding:0;width:22%'><label for='dumpLen' style='display:block;margin:0 0 6px'>Length</label><input id='dumpLen' name='len' type='text' value='4096' required></td>"));
  server.sendContent(F("</tr></table>"));
  server.sendContent(F("<button type='submit'>Dump EPROM</button></form>"));
#endif

  server.sendContent(F("<h2>Status</h2><ul>"));
  server.sendContent(F("<li>Wi-Fi mode: <code>"));
  server.sendContent(htmlEscape(wifiModeLabel));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>IP address: <code>"));
  server.sendContent(htmlEscape(wifiIpAddress));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>AP SSID: <code>"));
  server.sendContent(htmlEscape(String(kApSsid)));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>Last file: <code>"));
  server.sendContent(htmlEscape(lastFilename.length() ? lastFilename : String("none")));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>Last file size: <code id='stLastFileBytes'>"));
  server.sendContent(String(lastFileBytes));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>Last upload start: <code>"));
  server.sendContent(htmlEscape(formatAddressForInput(lastUploadStartAddr)));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("</ul>"));

  server.sendContent(F("<div class='messages'>"));
  if (message.length() > 0) {
    server.sendContent(F("<div class='msg'>"));
    server.sendContent(htmlEscape(message));
    server.sendContent(F("</div>"));
  }
  server.sendContent(F("<div id='waitNotice' class='msg wait' style='display:none'>Upload in progress. Please wait until the page reports completion.</div>"));
  server.sendContent(F("<div id='liveNotice' class='msg ok' style='display:none'></div>"));
  server.sendContent(F("</div>"));

  sendFsDirectoryHtmlStreamed();

  server.sendContent(F("<div style='margin-top:18px;padding-top:12px;border-top:1px solid #334155'>"));
  server.sendContent(F("<form method='POST' action='/reset-settings' onsubmit=\"return confirm('Clear saved defaults?');\">"));
  server.sendContent(F("<button type='submit' style='background:#f59e0b;color:#111827'>Clear saved defaults</button>"));
  server.sendContent(F("</form></div>"));

  server.sendContent(F("<script>"));
  server.sendContent(F("(()=>{const form=document.getElementById('uploadForm');const btn=document.getElementById('uploadButton');const wait=document.getElementById('waitNotice');const live=document.getElementById('liveNotice');const fileInput=document.getElementById('binfile');const uploadName=document.getElementById('uploadName');if(!form||!btn||!wait||!live){return;}form.addEventListener('submit',()=>{if(uploadName&&fileInput&&fileInput.files&&fileInput.files[0]){uploadName.value=fileInput.files[0].name||'';}btn.disabled=true;btn.textContent='Uploading...';wait.style.display='block';});const tick=()=>{fetch('/status',{cache:'no-store'}).then(r=>r.json()).then(s=>{const stLast=document.getElementById('stLastFileBytes');const stProgress=document.getElementById('stProgress');const stTotal=document.getElementById('stTotalBytes');if(stLast){stLast.textContent=s.lastFileBytes;}if(stProgress){stProgress.textContent=s.streamOffset+' / '+s.stagedFileBytes;}if(stTotal){stTotal.textContent=s.totalBytesSent;}}).catch(()=>{});};tick();setInterval(tick,1000);})();"));
  server.sendContent(F("</script></div></body></html>"));
  server.sendContent("");
}

void handleStreamFile() {
  if (!fsMounted) {
    pendingMessage = F("Cannot stream: LittleFS is not mounted.");
    redirectToRoot();
    return;
  }

  if (uploadInProgress) {
    pendingMessage = F("Cannot stream: upload is active.");
    redirectToRoot();
    return;
  }

  if (!server.hasArg("path")) {
    pendingMessage = F("Cannot stream: missing file path.");
    redirectToRoot();
    return;
  }

  const String path = normalizeFsPath(server.arg("path"));
  if (!isValidFsPath(path) || !LittleFS.exists(path)) {
    pendingMessage = F("Cannot stream: selected file not found.");
    redirectToRoot();
    return;
  }

  uint32_t startAddr = 0;
  if (server.hasArg("start")) {
    if (!parseUnsignedValue(server.arg("start"), startAddr)) {
      pendingMessage = F("Cannot stream: invalid start address.");
      redirectToRoot();
      return;
    }
  } else {
    loadStartAddressForFile(path, startAddr);
  }

  File f = LittleFS.open(path, "r");
  if (!f) {
    pendingMessage = F("Cannot stream: failed to open file.");
    redirectToRoot();
    return;
  }

  currentFilePath = path;
  stagedFileBytes = f.size();
  lastFileBytes = stagedFileBytes;
  lastFilename = path;
  f.close();

  if (stagedFileBytes == 0) {
    pendingMessage = F("Cannot stream: file is empty.");
    redirectToRoot();
    return;
  }

  if (!startPlaybackFromStaging(startAddr)) {
    pendingMessage = F("Cannot stream: failed to send file.");
    redirectToRoot();
    return;
  }

  if (!saveStartAddressForFile(path, startAddr)) {
    pendingMessage = F("Selected file sent, but start address could not be saved.");
    redirectToRoot();
    return;
  }

  lastFilename = baseNameFromPath(path);
  lastUploadStartAddr = startAddr;
  if (!saveGlobalSettings()) {
    pendingMessage = F("Selected file sent, but global settings could not be saved.");
    redirectToRoot();
    return;
  }

  pendingMessage = F("Selected file sent.");
  redirectToRoot();
}

void handleDeleteFile() {
  if (!fsMounted) {
    pendingMessage = F("Cannot delete: LittleFS is not mounted.");
    redirectToRoot();
    return;
  }

  if (!server.hasArg("path")) {
    pendingMessage = F("Cannot delete: missing file path.");
    redirectToRoot();
    return;
  }

  const String path = normalizeFsPath(server.arg("path"));
  if (!isValidFsPath(path) || !LittleFS.exists(path)) {
    pendingMessage = F("Cannot delete: selected file not found.");
    redirectToRoot();
    return;
  }

  if (!LittleFS.remove(path)) {
    pendingMessage = F("Delete failed.");
    redirectToRoot();
    return;
  }

  deleteStartAddressIniForFile(path);

  if (path == currentFilePath) {
    currentFilePath = String();
    stagedFileBytes = 0;
    streamOffset = 0;
    lastFileBytes = 0;
    lastFilename = String();
  }

  pendingMessage = F("File deleted.");
  redirectToRoot();
}

void handleResetSettings() {
  if (!fsMounted) {
    pendingMessage = F("Cannot clear defaults: LittleFS is not mounted.");
    redirectToRoot();
    return;
  }

  if (LittleFS.exists(kGlobalSettingsPath) && !LittleFS.remove(kGlobalSettingsPath)) {
    pendingMessage = F("Failed to clear saved defaults.");
    redirectToRoot();
    return;
  }

  if (LittleFS.exists(kLegacyGlobalSettingsPath) && !LittleFS.remove(kLegacyGlobalSettingsPath)) {
    pendingMessage = F("Failed to clear saved defaults.");
    redirectToRoot();
    return;
  }

  lastFilename = String();
  lastUploadStartAddr = 0;
  webUploadStartAddr = 0;
  staSsid = String(kStaSsid);
  staPassword = String(kStaPassword);
  pendingMessage = F("Saved defaults cleared.");
  redirectToRoot();
}

void handleDumpEprom() {
#ifdef GODIL_SPI
  if (!fsMounted) {
    pendingMessage = F("Cannot dump: LittleFS is not mounted.");
    redirectToRoot();
    return;
  }

  if (uploadInProgress) {
    pendingMessage = F("Cannot dump: upload is active.");
    redirectToRoot();
    return;
  }

  if (!server.hasArg("filename") || !server.hasArg("start") || !server.hasArg("len")) {
    pendingMessage = F("Cannot dump: missing filename/start/len.");
    redirectToRoot();
    return;
  }

  const String sanitizedName = sanitizeUploadFilename(server.arg("filename"));
  const String dumpPath = normalizeFsPath(sanitizedName);

  uint32_t startAddr = 0;
  uint32_t len32 = 0;
  if (!parseUnsignedValue(server.arg("start"), startAddr) ||
      !parseUnsignedValue(server.arg("len"), len32) || len32 == 0 || len32 > 0xFFFFUL) {
    pendingMessage = F("Cannot dump: invalid start or len.");
    redirectToRoot();
    return;
  }

  const uint16_t len = static_cast<uint16_t>(len32);
  if (!writeEPROMtoFile(dumpPath, startAddr, len)) {
    pendingMessage = F("EPROM dump failed. See serial output for details.");
    redirectToRoot();
    return;
  }

  saveStartAddressForFile(dumpPath, startAddr);

  lastFilename = baseNameFromPath(dumpPath);
  lastFileBytes = len;
  pendingMessage = F("EPROM dump written to ");
  pendingMessage += htmlEscape(dumpPath);
  pendingMessage += F(" (");
  pendingMessage += String(len);
  pendingMessage += F(" bytes).");
  redirectToRoot();
#else
  pendingMessage = F("EPROM dump is only available in GODIL mode.");
  redirectToRoot();
#endif
}

void handleStatus() {
  String json;
  json.reserve(220);
  json += F("{");
  json += F("\"streamOffset\":");
  json += String(streamOffset);
  json += F(",\"stagedFileBytes\":");
  json += String(stagedFileBytes);
  json += F(",\"lastFileBytes\":");
  json += String(lastFileBytes);
  json += F(",\"totalBytesSent\":");
  json += String(totalBytesSent);
  json += F("}");

  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server.send(200, "application/json", json);
}

void handleDownloadFile() {
  if (!fsMounted) {
    server.send(503, "text/plain", "LittleFS is not mounted.");
    return;
  }

  if (!server.hasArg("path")) {
    server.send(400, "text/plain", "Missing file path.");
    return;
  }

  const String path = normalizeFsPath(server.arg("path"));
  if (!isValidFsPath(path) || !LittleFS.exists(path)) {
    server.send(404, "text/plain", "File not found.");
    return;
  }

  File downloadFile = LittleFS.open(path, "r");
  if (!downloadFile) {
    server.send(500, "text/plain", "Failed to open file.");
    return;
  }

  String downloadName = baseNameFromPath(path);
  if (downloadName.length() == 0) {
    downloadName = F("download.bin");
  }

  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Content-Disposition",
                    String("attachment; filename=\"") + downloadName + String("\""));
  server.streamFile(downloadFile, "application/octet-stream");
  downloadFile.close();
}
// ##############################################################################
//
//     #     # ####### ######     #     # ######  #       #######    #    ######  
//     #  #  # #       #     #    #     # #     # #       #     #   # #   #     # 
//     #  #  # #       #     #    #     # #     # #       #     #  #   #  #     # 
//     #  #  # #####   ######     #     # ######  #       #     # #     # #     # 
//     #  #  # #       #     #    #     # #       #       #     # ####### #     # 
//     #  #  # #       #     #    #     # #       #       #     # #     # #     # 
//      ## ##  ####### ######      #####  #       ####### ####### #     # ######  
//                                                                                
// ##############################################################################
// WEB UPLOAD HANDLERS
// ##############################################################################

// Handles multipart upload events and stores incoming data to staging file.
void handleUpload() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    setUploadLed(true);
    #ifdef USE_DY1_DISPLAY
      set_static_message(F("upl"));
    #endif
    currentUploadFsError = false;

    DBG_PRINT(F("Upload start: "));
    DBG_PRINTLN(upload.filename);

    uploadInProgress = true;
    currentUploadFsError = !fsMounted;
    currentUploadStartArgInvalid = false;
    webUploadStartAddr = 0;
    const String formUploadName = server.hasArg("uploadName") ? server.arg("uploadName") : String();
    lastFilename = selectWebUploadName(upload.filename, formUploadName);
    currentFilePath = makeWebUploadFilePath(lastFilename);
    lastFileBytes = 0;
    stagedFileBytes = 0;
    streamOffset = 0;
    clearDataBus();

    if (uploadStagingFile) {
      uploadStagingFile.close();
    }

    if (!currentUploadFsError) {
      LittleFS.remove(currentFilePath);
      uploadStagingFile = LittleFS.open(currentFilePath, "w");
      if (!uploadStagingFile) {
        currentUploadFsError = true;
        DBG_PRINTLN(F("Upload start error: failed to open staging file."));
      }
    } else {
      DBG_PRINTLN(F("Upload start error: filesystem not mounted."));
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (!currentUploadFsError) {
      const size_t written = uploadStagingFile.write(upload.buf, upload.currentSize);
      stagedFileBytes += written;
      DBG_PRINT(F("Upload write chunk="));
      DBG_PRINT(upload.currentSize);
      DBG_PRINT(F(", written="));
      DBG_PRINT(written);
      DBG_PRINT(F(", total="));
      DBG_PRINTLN(stagedFileBytes);
      if (written != upload.currentSize) {
        currentUploadFsError = true;
        DBG_PRINTLN(F("Upload write error: short write to staging file."));
      }

      // Keep the HTTP/TCP stack responsive while receiving upload chunks.
      delay(0);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadStagingFile) {
      uploadStagingFile.close();
    }

    uploadInProgress = false;

    if (!currentUploadFsError && stagedFileBytes > 0) {
      lastFileBytes = stagedFileBytes;
    } else {
      stagedFileBytes = 0;
      streamOffset = 0;
      clearDataBus();
    }

    DBG_PRINT(F("Upload end: bytes="));
    DBG_PRINT(stagedFileBytes);
    DBG_PRINT(F(", fsError="));
    DBG_PRINT(currentUploadFsError ? F("true") : F("false"));
    DBG_PRINTLN();

    setUploadLed(false);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    DBG_PRINTLN(F("Upload aborted."));
    if (uploadStagingFile) {
      uploadStagingFile.close();
    }
    if (currentFilePath.length() > 0) {
      LittleFS.remove(currentFilePath);
    }
    stagedFileBytes = 0;
    streamOffset = 0;
    clearDataBus();
    uploadInProgress = false;
    setUploadLed(false);
  }
}

// Finishes POST request and redirects with a short status message.
void handleUploadDone() {
  String message;
  if (currentUploadFsError) {
    message = F("Upload failed: staging file write/read error.");
  } else if (stagedFileBytes == 0) {
    message = F("Upload received no data.");
  } else {
    currentUploadStartArgInvalid = false;
    webUploadStartAddr = 0;
    if (!server.hasArg("start") || !parseUnsignedValue(server.arg("start"), webUploadStartAddr)) {
      currentUploadStartArgInvalid = true;
      message = F("Upload failed: invalid start address.");
    } else if (!saveStartAddressForFile(currentFilePath, webUploadStartAddr)) {
      message = F("Upload failed: could not save start address.");
    } else if (!startPlaybackFromStaging(webUploadStartAddr)) {
      message = F("Upload failed: could not stream staged file.");
    } else {
    lastFilename = baseNameFromPath(currentFilePath);
    lastUploadStartAddr = webUploadStartAddr;
    if (!saveGlobalSettings()) {
      message = F("Upload failed: could not save global settings.");
      pendingMessage = message;
      redirectToRoot();
      return;
    }
    message = F("Upload received. ");
    message += String(stagedFileBytes);
    message += F(" bytes processed from ");
    message += htmlEscape(lastFilename.length() ? lastFilename : String("unknown file"));
    }
  }
  #ifdef USE_DY1_DISPLAY
    set_static_message(F("rdy"));
  #endif

  pendingMessage = message;
  redirectToRoot();
}

// ##############################################################################
//
//      #####  ####### ######  #     # ####### ######     ### #     # ### ####### 
//     #     # #       #     # #     # #       #     #     #  ##    #  #     #    
//     #       #       #     # #     # #       #     #     #  # #   #  #     #    
//      #####  #####   ######  #     # #####   ######      #  #  #  #  #     #    
//           # #       #   #    #   #  #       #   #       #  #   # #  #     #    
//     #     # #       #    #    # #   #       #    #      #  #    ##  #     #    
//      #####  ####### #     #    #    ####### #     #    ### #     # ###    # 
//
// ##############################################################################
// WEB SERVER INIT
// ##############################################################################
                                                                              
void serverInit() {
#if defined(STA_MODE)
  WiFi.mode(WIFI_STA);
  WiFi.begin(staSsid.c_str(), staPassword.c_str());
  Serial.print(F("Wi-Fi Connecting to SSID: "));
  Serial.print(staSsid);
  Serial.print(F(" ."));

  unsigned long startMs = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startMs) < 5000UL) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    Serial.print(F("."));
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiModeLabel = F("STA");
    wifiIpAddress = WiFi.localIP().toString();
  } else {
    wifiModeLabel = F("AP (fallback from STA)");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(kApSsid, kApPassword);
    wifiIpAddress = WiFi.softAPIP().toString();
    // Blink LED to indicate AP mode fallback due to STA connection failure.
    for (int i = 0; i < 10; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      digitalWrite(LED_SENDDATA, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(LED_SENDDATA, HIGH);
      delay(100);
    }
    digitalWrite(LED_SENDDATA, LOW);
  }
#else
  Serial.print(F("Wi-Fi in AP mode"));
  Serial.println(kApSsid);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPassword);
  wifiModeLabel = F("AP ");
  wifiIpAddress = WiFi.softAPIP().toString();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/download-file", HTTP_GET, handleDownloadFile);
  server.on("/stream-file", HTTP_POST, handleStreamFile);
  server.on("/delete-file", HTTP_POST, handleDeleteFile);
  server.on("/reset-settings", HTTP_POST, handleResetSettings);
  server.on("/dump-eprom", HTTP_POST, handleDumpEprom);
  server.on("/upload", HTTP_POST, handleUploadDone, handleUpload);
  server.begin();
}


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

// Opens the staged file and sends it in one blocking pass.
bool startPlaybackFromStaging(uint32_t startAddr) {
  if (!fsMounted || stagedFileBytes == 0 || currentFilePath.length() == 0 || !LittleFS.exists(currentFilePath)) {
    Serial.println(F("No file to send."));
    return false;
  }

  File playbackFile = LittleFS.open(currentFilePath, "r");
  if (!playbackFile) {
    Serial.println(F("Failed to open playback file."));
    return false;
  }

  const uint32_t playbackStartMs = millis();
  size_t playbackBytesSent = 0;
#if defined(DEBUG)
  DBG_PRINT(F("Playback start: file="));
  DBG_PRINT(currentFilePath);
  DBG_PRINT(F(", addr="));
  DBG_PRINT(startAddr);
  DBG_PRINT(F(" (0x"));
  Serial.print(startAddr, HEX);
  DBG_PRINT(F("), bytes="));
  DBG_PRINTLN(stagedFileBytes);
#endif

  streamOffset = 0;
  setUpSendLed(true);
  #ifdef USE_DY1_DISPLAY
    set_static_message(F("rpl"));
  #endif

  while (playbackFile.available()) {
    const int nextByte = playbackFile.read();
    if (nextByte < 0) {
      break;
    }

    outputByte(static_cast<uint8_t>(nextByte), startAddr + static_cast<uint32_t>(streamOffset));
    ++streamOffset;
  ++playbackBytesSent;
    ++totalBytesSent;

    if ((streamOffset & 0x1F) == 0) {
      delay(0);
    }
  }

  playbackFile.close();
  Serial.println(F("Finished playback."));
#if defined(DEBUG)
  const uint32_t elapsedMs = millis() - playbackStartMs;
  DBG_PRINT(F("Playback done: file="));
  DBG_PRINT(currentFilePath);
  DBG_PRINT(F(", bytes="));
  DBG_PRINT(playbackBytesSent);
  DBG_PRINT(F(", elapsed_ms="));
  DBG_PRINT(elapsedMs);
  DBG_PRINT(F(", rate_Bps="));
  if (elapsedMs > 0) {
    DBG_PRINTLN(static_cast<unsigned long>((static_cast<uint32_t>(playbackBytesSent) * 1000UL) / elapsedMs));
  } else {
    DBG_PRINTLN(F("n/a"));
  }
#endif
  setUpSendLed(false);
  clearDataBus();
  delay(0);
  return true;
}

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

// Handles serial upload command.
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
  DBG_PRINT(F("Serial upload path: "));
  DBG_PRINTLN(currentFilePath);
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
      delay(0);
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


// Processes single-character serial commands (r, c, i, x, n, u, and d).
void processSerialCommands() {
  String dumpLine;
  String filename;
  String wifiValue;
  uint32_t startAddr = 0;
  uint16_t len = 0;
  while (Serial.available() > 0) {
    const char cmd = static_cast<char>(Serial.read());
    Serial.println();
    switch (cmd) {
      case 'r':
      case 'R':
        startPlaybackFromStaging(resolveStartAddressForPath(currentFilePath));
        break;
      case 'i':
      case 'I':
        printFileInfo();
        break;
      case 'l':
      case 'L':
        listLittleFsEntries();
        break;
      case 'h':
      case 'H':
        printWebInfo();
        break;
      case 'x':
      case 'X':
        Serial.println(F("Ready."));
        break;
      case 't':
      case 'T':
        testSPItransfer();
        #ifdef USE_DY1_DISPLAY
          test_display();
        #endif
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
          DBG_PRINT(F("Serial filename accepted: "));
          DBG_PRINTLN(pendingSerialFilename);
        }
        break;
      case 'w':
      case 'W':
        if (!readSerialLine(wifiValue, kSerialUploadTimeoutMs * 20U)) {
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
        if (!readSerialLine(wifiValue, kSerialUploadTimeoutMs * 20U)) {
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
      case 'd':
      case 'D':
        #ifdef GODIL_SPI
          if (!readSerialLine(dumpLine, kSerialUploadTimeoutMs * 20U)) {
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
        break;
    }
  }
}

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

  SPI.begin();
  SPI.setFrequency(10000000);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, HIGH);

#ifdef GODIL_SPI
  pinMode(LED_UPLOAD, OUTPUT);
  digitalWrite(LED_UPLOAD, LOW);
#else
  pinMode(STROBE_PIN, OUTPUT);
  digitalWrite(STROBE_PIN, HIGH); 
#endif
#ifdef USE_DY1_DISPLAY
  clear_disp(0);
  pinMode(DY1_LATCH_PIN, OUTPUT);
  digitalWrite(DY1_LATCH_PIN, LOW);
  set_static_message(F("rst"));
#endif

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  pinMode(LED_SENDDATA, OUTPUT);
  digitalWrite(LED_SENDDATA, LOW);
  Serial.println();
  #ifdef GODIL_SPI
    Serial.println(F("GODIL Binary Uploader by Carsten Meyer 7/2026"));
  #else
    Serial.println(F("ESIM Binary Uploader by Carsten Meyer 7/2026"));
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
  #endif

  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: LittleFS init failed"));
    fsMounted = false;
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
  printFileInfo();
  startPlaybackFromStaging(resolveStartAddressForPath(currentFilePath));
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
      delay(500); // additional delay to make the last digit visible for a bit longer
      set_static_message(wifiModeLabel);
      delay(500); 
      set_static_message(F("on "));
    #endif
    printWebInfo();
    Serial.println(F("Ready."));
  #else
    #ifdef USE_DY1_DISPLAY
      set_static_message(F("off"));
    #endif
    Serial.println(F("Web server disabled."));
    Serial.println(F("Ready."));
  #endif
}

// Main service loop for serial commands, HTTP handling, and playback.
void loop() {
  processSerialCommands();
  #ifdef USE_WEB_SERVER
    server.handleClient();
  #endif
}
