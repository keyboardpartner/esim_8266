#ifndef GLOBALVARS_H
#define GLOBALVARS_H

#include <Arduino.h>

// Uncomment or define STA_MODE to connect to an existing Wi-Fi network instead of starting an access point.
#define STA_MODE

// uncomment to use Web Server
#define USE_WEB_SERVER

// uncomment to use  DY1 Display
// #define USE_DY1_DISPLAY

// uncomment to use  TFT 0.96" Display (SPI)
#define USE_TFT_DISPLAY

#ifdef USE_TFT_DISPLAY
  #include <TFT_eSPI.h>
  #include "Free_Fonts.h"
  TFT_eSPI tft = TFT_eSPI();       // Invoke custom library as global

  // https://rgbcolorpicker.com/565
  #define TFT_MEDGREY  0x8410
  #define TFT_DIALOGGREY 0x736e     // Window background color
  #define TFT_CHARCOAL 0x2124
 
  #define TFT_V_OFFS  24 // needed for 0.96" TFT display with 160x80 resolution, to center the content vertically

  #define DISPLAY_CENTER_X  80
  #define DISPLAY_CENTER_Y  (40 + TFT_V_OFFS) // 40 + vertical offset for 0.96" TFT display with 160x80 resolution

  // msgType =0/16 "i" in blau, =1/17 "?" in blau, =2/18 "!" in rot
  enum DialogBoxType {
    DB_INFO = 0,
    DB_REQUEST = 1,
    DB_ERROR = 2,
  };

  void fillShadowRect() {
    tft.drawRect(0, TFT_V_OFFS, 160, 80, TFT_WHITE);
    tft.fillRectVGradient(1, TFT_V_OFFS + 1, 158, 78, TFT_DIALOGGREY, TFT_CHARCOAL);
  }

#endif


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

constexpr const char *versionString = "0.92";

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
 #endif

#ifdef JTAG_SPARTAN6
  #define LATCH_PIN 15 // FPGA SPI /SS
  #include "jtag_send.h"
  constexpr const char *kApSsid = "FPGA Uploader";
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

constexpr const char *kChipTypeNames[] = {
  "2716",  // 0
  "2732",  // 1
  "2764",  // 2
  "27128", // 3
  "27256", // 4
  "27512", // 5
  "2532",  // 6
  "2364",  // 7
  "6116",  // 8
  "6264",  // 9
  "62256", // 10
  "KCPB3", // 11
  "KCPB6", // 12
  "(none)"
};
constexpr uint32_t kMaxBytesToTransfer[] = {
  2048,   // 2716
  4096,   // 2732
  8192,   // 2764
  16384,  // 27128
  32768,  // 27256
  65536,  // 27512
  4096,   // 2532
  8192,   // 2364
  2048,   // 6116
  8192,   // 6264
  32768,  // 62256
  4096,   // KCPB3  - 1024 18 Bit Words = 4096 Bytes
  16384,  // KCPB6  - 4096 18 Bit Words = 16384 Bytes
  4096,   // none
};

constexpr size_t kChipTypeCount = sizeof(kChipTypeNames) / sizeof(kChipTypeNames[0]);
constexpr uint32_t kInvalidChipTypeIndex = 13; // default to 13, which is an invalid index, meaning no chip type selected
uint32_t currentChipTypeIndex = kInvalidChipTypeIndex; // default to 13, which is an invalid index, meaning no chip type selected

enum WifiMode {
  WifiMode_Unknown = 0,
  WifiMode_STA,
  WifiMode_AP
};

WifiMode currentWifiMode = WifiMode_Unknown;
String wifiModeLabel = "AP";
String wifiIpAddress = "192.168.4.1";

String lastFilename;
size_t lastFileBytes = 0;
bool uploadInProgress = false;
bool fsMounted = false;

File uploadStagingFile;
size_t stagedFileBytes = 0;
size_t streamOffset = 0;
String currentFilePath;
String pendingSerialFilename;

uint32_t lastUploadStartAddr = 0;
uint32_t lastStreamedStartAddr = 0;

String staSsid = String(kStaSsid);
String staPassword = String(kStaPassword);

bool currentUploadFsError = false;
String pendingMessage;
bool likelyFreshFsImage = false;
String startupFpgaPath = "/fpga_main.bit";
bool autoplayEnabled = false;
uint8_t port0value = 0;
uint8_t port1value = 0;


unsigned long msgTimeout = -1; // no timeout by default


void drawMsgBox(const String &title, const String &message, DialogBoxType msgType, uint32_t timeoutMs = 1000) {
  #ifdef USE_TFT_DISPLAY
    fillShadowRect();
    uint16_t icon_color;
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setFreeFont(FF18);
    tft.setTextDatum(TC_DATUM);
    if (message.length() == 0) {
      tft.drawString(title, 100, TFT_V_OFFS + 32);
    } else {
      tft.drawString(title, 100, TFT_V_OFFS + 14);
      tft.drawString(message, 100, TFT_V_OFFS + 42);
    }
    switch (msgType) {
    case DB_REQUEST:
      icon_color = TFT_GREEN;
      break;
    case DB_ERROR:
      icon_color = TFT_RED;
      break;
    default:
      icon_color = TFT_BLUE;
      break;
    }
    tft.setTextDatum(MC_DATUM); // middle center text datum
    tft.fillRoundRect(16, DISPLAY_CENTER_Y - 18, 26, 36, 4, icon_color); // center_x - 120 +
    tft.setTextColor(TFT_WHITE, icon_color);
    tft.setFreeFont(FF22);
    int16_t icon_x = 28; // adjust center_x for icon position
    int16_t icon_y = DISPLAY_CENTER_Y - 2;    // nudge up
    switch (msgType) {
    case DB_INFO:
      tft.drawString("i", icon_x, icon_y);
      break;
    case DB_REQUEST:
      tft.drawString("?", icon_x, icon_y);
      break;
    case DB_ERROR:
      tft.drawString("!", icon_x, icon_y);
      break;
    }
  #endif
  msgTimeout = millis() + timeoutMs;
  if (msgType == DB_ERROR) {
    delay(2000); // wait 2 seconds for error messages
    msgTimeout = -1; // reset timeout to avoid repeated drawing
  }
} 

void drawStringBox(const String &message1, const String &message2 = "", uint32_t timeoutMs = 1000) {
  #ifdef USE_TFT_DISPLAY
    fillShadowRect();
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    if (message1.length() < 10) {
      tft.setFreeFont(FF18);
    } else {
      tft.setFreeFont(FF17);
    }
    if (message2.length() > 0) {
      if (message2.length() < 10) {
        tft.setFreeFont(FF18);
      } else {
        tft.setFreeFont(FF17);
      }
      tft.drawString(message1, DISPLAY_CENTER_X, DISPLAY_CENTER_Y - 12);
      tft.drawString(message2, DISPLAY_CENTER_X, DISPLAY_CENTER_Y + 12);
    } else {
      tft.drawString(message1, DISPLAY_CENTER_X, DISPLAY_CENTER_Y);
    }
  #endif
  msgTimeout = millis() + timeoutMs;
}

void drawStatusBox() {
  #ifdef USE_TFT_DISPLAY
    fillShadowRect();
    tft.setTextColor(TFT_YELLOW);
    tft.setTextSize(1);
    tft.setFreeFont(FF17);
    int16_t y= TFT_V_OFFS + 18;
    int16_t line_height = 18;
    tft.setCursor(5, y);
    if (currentWifiMode == WifiMode_STA) {
      tft.print(WiFi.localIP().toString());
    } else {
      tft.print(WiFi.softAPIP().toString());
    }
    y += line_height;
    tft.setTextColor(TFT_CYAN);
    tft.setCursor(5, y);
    tft.printf("FPGA: %08X", fpgaVersion);
    y += line_height;
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(5, y);
    if (currentFilePath.length() > 18) {
      tft.setTextFont(1);
      tft.setCursor(5, y-10);
    }
    tft.print(currentFilePath);
    tft.setFreeFont(FF17);
    tft.setTextColor(TFT_GREEN);
    y += line_height;
    tft.setCursor(5, y);
    tft.print(kChipTypeNames[currentChipTypeIndex]);
    tft.setCursor(DISPLAY_CENTER_X, y);
    tft.print("0x"+String(lastStreamedStartAddr, HEX));
    msgTimeout = -1; // reset timeout to avoid repeated drawing
  #endif
}

void readyMessage() {
  #ifdef USE_DY1_DISPLAY
    // Anzeige "rdy" auf OHO-Display
    set_letter(0, 'r');
    set_letter(1, 'd');
    set_letter(2, 'y');
    spi_send_displ_arr();
    msgTimeout = millis() + 1000; // reset timeout to avoid repeated drawing
  #endif
  #ifdef USE_TFT_DISPLAY
    drawMsgBox(F("Ready"), F(""), DB_INFO, 1000);
  #endif
}

#endif  // GLOBALVARS_H