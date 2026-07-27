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

// Upload a binary file to the ESP8266 over Wi-Fi and stream its bytes to a 74HC595 shift register using SPI
// (LSB first, 8-bit parallel output, with a strobe signal to latch the outputs).
// Useful for EPROM simulators loaded by a Centronics-like parallel interface,
// or other applications that require streaming bytes to a shift register.

// 74HC595 SER (DS) to MOSI (GPIO13 / D7), SRCLK (SH_CP) to SCLK (GPIO14 / D5).
// RCLK (ST_CP latch) uses GPIO 5 (D1). Tie /OE low and /SRCLR high for always-enabled output.

// Serial commands: r=replay staged file, c=cancel streaming, i=print info,
// n=next serial upload filename terminated by CR,
// u=lenLo,lenHi,data... upload payload by serial (LE length, timeout-protected).

// The associated sending app ESIM.EXE uses the following protocol:
// Added waitForAck(...) with a 1-second timeout per wait.
// Waits for ASCII ACK byte 0x06.
// Returns error on timeout or read failure.
// Still prints incoming text lines and tracks ERROR: messages while waiting.
// Upload flow in main is now:
// Send u command + 16-bit length word.
// Wait for ACK (1 second timeout).
// Send data in chunks of 128 bytes (last chunk can be smaller).
// After each chunk, wait for ACK (1 second timeout).
// Compute 16-bit checksum (sum of all data bytes modulo 65536).
// Send checksum as 16-bit word LSB first.
// Wait for final ACK (1 second timeout).
// On any timeout or NAK, the app exits with an explicit error message indicating at which stage ACK timed out.

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <SPI.h>
#include <cstdio>

namespace {

// Uncomment or define STA_MODE to connect to an existing Wi-Fi network instead of starting an access point.
#define STA_MODE
// Uncomment to enable verbose serial debug output.
#define DEBUG

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
constexpr const char *kApSsid = "ESIM Uploader";
constexpr const char *kApPassword = "0000";
constexpr const char *kStaSsid = "KeyboardPartner";
constexpr const char *kStaPassword = "z28hev111";

constexpr uint32_t strobeDelayMicros = 5;
constexpr size_t kStreamBatchBytes = 16;
constexpr const char *kIniFilePath = "/esim.ini";
constexpr const char *kUploadFilePrefix = "/esim_";
constexpr size_t kMaxFsPathLength = 31;
constexpr uint32_t maxBytesToTransfer = 65536;
constexpr uint32_t kSerialUploadTimeoutMs = 200;
constexpr uint8_t kSerialAckByte = 0x06;
constexpr uint8_t kSerialNakByte = 0x15;
constexpr uint16_t kSerialAckChunkBytes = 128;

constexpr uint8_t kLatchPin = 15;
constexpr uint8_t kStrobePin = 5;

ESP8266WebServer server(80);
String wifiModeLabel = "AP";
String wifiIpAddress = "192.168.4.1";

String lastFilename;
size_t lastFileBytes = 0;
size_t totalBytesSent = 0;
size_t uploadCount = 0;
bool uploadInProgress = false;
bool streamInProgress = false;
bool fsMounted = false;

File uploadStagingFile;
File playbackFile;
size_t stagedFileBytes = 0;
size_t streamOffset = 0;
uint32_t nextUploadId = 1;
String currentFilePath;
String pendingSerialFilename;

bool currentUploadRejected = false;
bool currentUploadFsError = false;
String pendingMessage;

String htmlEscape(const String &input);
bool startPlaybackFromStaging(bool allowDuringUpload = false);

void redirectToRoot() {
  server.sendHeader("Location", "/", true);
  server.send(303, "text/plain", "");
}

String makeUploadFilePath(uint32_t id) {
  char buf[24];
  std::snprintf(buf, sizeof(buf), "%s%06lu.bin", kUploadFilePrefix,
                static_cast<unsigned long>(id));
  return String(buf);
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

String sanitizeIniValue(const String &input) {
  String out;
  out.reserve(input.length());
  for (char c : input) {
    if (c != '\r' && c != '\n') {
      out += c;
    }
  }
  return out;
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

void persistUploadInfoToIni(const String &sourceTag) {
  if (!fsMounted) {
    return;
  }

  File iniFile = LittleFS.open(kIniFilePath, "a");
  if (!iniFile) {
    Serial.println(F("ERROR: failed to open esim.ini"));
    return;
  }

  iniFile.printf("last_path=%s\n", sanitizeIniValue(currentFilePath).c_str());
  iniFile.printf("last_name=%s\n", sanitizeIniValue(lastFilename).c_str());
  iniFile.printf("last_size=%lu\n", static_cast<unsigned long>(lastFileBytes));
  iniFile.printf("upload_count=%lu\n", static_cast<unsigned long>(uploadCount));
  iniFile.printf("next_id=%lu\n", static_cast<unsigned long>(nextUploadId));
  iniFile.printf("file.%06lu=%s|%s|%lu|%s\n", static_cast<unsigned long>(nextUploadId - 1),
                 sanitizeIniValue(currentFilePath).c_str(),
                 sanitizeIniValue(lastFilename).c_str(),
                 static_cast<unsigned long>(lastFileBytes),
                 sanitizeIniValue(sourceTag).c_str());
  iniFile.close();
}

void loadStateFromIni() {
  if (!fsMounted || !LittleFS.exists(kIniFilePath)) {
    return;
  }

  File iniFile = LittleFS.open(kIniFilePath, "r");
  if (!iniFile) {
    return;
  }

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

    if (key == F("last_path")) {
      currentFilePath = value;
    } else if (key == F("last_name")) {
      lastFilename = value;
    } else if (key == F("last_size")) {
      lastFileBytes = static_cast<size_t>(value.toInt());
      stagedFileBytes = lastFileBytes;
    } else if (key == F("upload_count")) {
      uploadCount = static_cast<size_t>(value.toInt());
    } else if (key == F("next_id")) {
      const uint32_t parsed = static_cast<uint32_t>(value.toInt());
      if (parsed > 0) {
        nextUploadId = parsed;
      }
    }
  }

  iniFile.close();
}

String buildFsDirectoryHtml() {
  String html;
  html.reserve(3200);
  html += F("<h2>LittleFS Files</h2>");

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
  html += F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Actions</th>");
  html += F("</tr></thead><tbody>");

  while (dir.next()) {
    const String path = normalizeFsPath(dir.fileName());
    if (path == kIniFilePath) {
      continue;
    }

    any = true;
    const size_t bytes = dir.fileSize();
    const String displayName = baseNameFromPath(path);

    html += F("<tr>");
    html += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937'><code>");
    html += htmlEscape(displayName.length() ? displayName : path);
    html += F("</code></td>");
    html += F("<td style='padding:6px 8px;text-align:right;border-bottom:1px solid #1f2937'>");
    html += String(bytes);
    html += F(" bytes</td>");
    html += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937;white-space:nowrap'>");

    html += F("<form method='POST' action='/stream-file' style='display:inline-block;margin:0 6px 0 0'>");
    html += F("<input type='hidden' name='path' value='");
    html += htmlEscape(path);
    html += F("'><button class='mini' type='submit'>Stream</button></form>");

    html += F("<form method='POST' action='/delete-file' style='display:inline-block;margin:0' onsubmit=\"return confirm('Delete file?');\">");
    html += F("<input type='hidden' name='path' value='");
    html += htmlEscape(path);
    html += F("'><button class='mini' type='submit'>Delete</button></form>");

    html += F("</td></tr>");
  }

  if (!any) {
    html += F("<tr><td colspan='3' style='padding:8px;color:#94a3b8'>No files present.</td></tr>");
  }

  html += F("</tbody></table>");
  return html;
}

// Prints current staged file and streaming state for serial debugging.
void printFileDebugInfo() {
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

  Serial.println(F("--- File Debug Info ---"));
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
  Serial.print(F("streamInProgress: "));
  Serial.println(streamInProgress ? F("true") : F("false"));
  Serial.print(F("streamOffset: "));
  Serial.println(streamOffset);
  Serial.print(F("lastFilename: "));
  Serial.println(lastFilename.length() ? lastFilename : String(F("none")));
  Serial.print(F("lastFileBytes: "));
  Serial.println(lastFileBytes);
  Serial.print(F("uploadInProgress: "));
  Serial.println(uploadInProgress ? F("true") : F("false"));
  Serial.println(F("-----------------------"));
#else
  Serial.println(F("Serial command 'i': DEBUG is disabled."));
#endif
}

void printWebInfo() {
  Serial.println();
  Serial.println(F("ESIM Web Uploader by cm 7/2026"));
  Serial.print(F("Wi-Fi mode: "));
  Serial.println(wifiModeLabel);
  Serial.print(F("Wi-Fi SSID: "));
  Serial.println(kStaSsid);
  Serial.print(F("IP address: "));
  Serial.println(wifiIpAddress);
  Serial.print(F("AP SSID: "));
  Serial.println(kApSsid);
  Serial.print(F("Last file: "));
  Serial.println(lastFilename.length() ? lastFilename : String(F("none")));
  Serial.print(F("Last file size: "));
  Serial.println(lastFileBytes);
  Serial.println(F("Ready."));
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

// Controls the onboard LED used as HTTP upload activity indicator.
void setUploadLed(bool on) {
  digitalWrite(LED_BUILTIN, on ? HIGH : LOW);
}

// Controls a dedicated LED that indicates active file playback/streaming.
void setUpSendLed(bool on) {
  digitalWrite(LED_SENDDATA, on ? HIGH : LOW);
}

// Shifts one byte to the 74HC595 using SPI and latches the new output state.
void setDataBus(uint8_t value) {
  digitalWrite(kLatchPin, LOW);
  SPI.transfer(value);
  digitalWrite(kLatchPin, HIGH);
}

// Clears all output bits on the shift register.
void clearDataBus() {
  setDataBus(0);
}

// Pulses the external strobe signal once.
void pulseStrobe() {
  // The 74HC595 output update happens on latch edge in setDataBus().
  delayMicroseconds(strobeDelayMicros);
  digitalWrite(kStrobePin, LOW);
  delayMicroseconds(strobeDelayMicros);
  digitalWrite(kStrobePin, HIGH);
}

// Sends one byte to outputs and applies the configured inter-byte delay.
void outputByte(uint8_t value) {
  setDataBus(value);
  pulseStrobe();
  delayMicroseconds(strobeDelayMicros);
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

// Builds the uploader web page with status fields and optional message.
String buildPage(const String &message = String()) {
  String page;
  page.reserve(5200);
  page += F("<!doctype html><html><head><meta charset='utf-8'>");
  page += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<title>ESIM Binary Uploader</title>");
  page += F("<style>");
  page += F("body{font-family:Arial,sans-serif;background:#0f172a;color:#e2e8f0;margin:0;padding:24px;}");
  page += F(".card{width:780px;max-width:calc(100vw - 48px);margin:0 auto;background:#111827;border:1px solid #334155;border-radius:16px;padding:24px;box-shadow:0 20px 50px rgba(0,0,0,.25);box-sizing:border-box;}h1{margin-top:0;font-size:28px;}p,li{line-height:1.5;}label{display:block;margin:16px 0 8px;}input[type=file]{display:block;width:100%;padding:12px;background:#0b1220;border:1px solid #334155;color:#e2e8f0;border-radius:10px;}button{margin-top:16px;background:#22c55e;border:0;color:#052e16;font-weight:700;padding:12px 18px;border-radius:10px;cursor:pointer;}button[disabled]{opacity:.65;cursor:not-allowed;}.mini{margin-top:0;padding:4px 8px;font-size:12px;}code{background:#0b1220;padding:2px 6px;border-radius:6px;}.muted{color:#94a3b8;}.messages{min-height:72px;margin:16px 0;display:flex;flex-direction:column;justify-content:flex-start;}.msg{margin:0;padding:12px 14px;background:#0b1220;border-left:4px solid #38bdf8;border-radius:8px;}.msg.wait{border-left-color:#f59e0b;}.msg.ok{border-left-color:#22c55e;}</style></head><body><div class='card'>");
  page += F("<h1>ESIM Binary Uploader</h1><h3>by Carsten Meyer 7/2026</h3>");

  page += F("<form id='uploadForm' method='POST' action='/upload' enctype='multipart/form-data'>");
  page += F("<label for='binfile'>Binary file</label><input id='binfile' name='binfile' type='file' accept='.bin,application/octet-stream' required>");
  page += F("<input id='uploadName' name='uploadName' type='hidden' value=''>");
  page += F("<button id='uploadButton' type='submit'>Upload and stream</button></form>");
  page += F("<div class='messages'>");
  if (message.length() > 0) {
    page += F("<div class='msg'>");
    page += htmlEscape(message);
    page += F("</div>");
  }
  page += F("<div id='waitNotice' class='msg wait' style='display:none'>Upload in progress. Please wait until the page reports completion.</div>");
  page += F("<div id='liveNotice' class='msg ok' style='display:none'></div>");
  page += F("</div>");
  page += F("<h2>Status</h2><ul>");
  page += F("<li>Wi-Fi mode: <code>");
  page += wifiModeLabel;
  page += F("</code></li>");
  page += F("<li>IP address: <code>");
  page += wifiIpAddress;
  page += F("</code></li>");
  page += F("<li>AP SSID: <code>");
  page += kApSsid;
  page += F("</code></li>");
  page += F("<li>Last file: <code>");
  page += htmlEscape(lastFilename.length() ? lastFilename : String("none"));
  page += F("</code></li>");
  page += F("<li>Completed uploads: <code id='stUploads'>");
  page += String(uploadCount);
  page += F("</code></li>");
  page += F("<li>Last file size: <code id='stLastFileBytes'>");
  page += String(lastFileBytes);
  page += F("</code></li>");
  page += F("</ul>");
  page += buildFsDirectoryHtml();
  page += F("<script>");
  page += F("(()=>{const form=document.getElementById('uploadForm');const btn=document.getElementById('uploadButton');const wait=document.getElementById('waitNotice');const live=document.getElementById('liveNotice');const fileInput=document.getElementById('binfile');const uploadName=document.getElementById('uploadName');if(!form||!btn||!wait||!live){return;}let prevStreaming=");
  page += (streamInProgress ? F("true") : F("false"));
  page += F(";let prevUploads=");
  page += String(uploadCount);
  page += F(";form.addEventListener('submit',()=>{if(uploadName&&fileInput&&fileInput.files&&fileInput.files[0]){uploadName.value=fileInput.files[0].name||'';}btn.disabled=true;btn.textContent='Uploading...';wait.style.display='block';});const tick=()=>{fetch('/status',{cache:'no-store'}).then(r=>r.json()).then(s=>{const stUploads=document.getElementById('stUploads');const stLast=document.getElementById('stLastFileBytes');const stStreaming=document.getElementById('stStreaming');const stProgress=document.getElementById('stProgress');const stTotal=document.getElementById('stTotalBytes');if(stUploads){stUploads.textContent=s.uploadCount;}if(stLast){stLast.textContent=s.lastFileBytes;}if(stStreaming){stStreaming.textContent=s.streamInProgress?'active':'idle';}if(stProgress){stProgress.textContent=s.streamOffset+' / '+s.stagedFileBytes;}if(stTotal){stTotal.textContent=s.totalBytesSent;}if(prevStreaming && !s.streamInProgress){live.textContent='Streaming finished for '+s.lastFileBytes+' bytes.';live.style.display='block';wait.style.display='none';}if(s.uploadCount>prevUploads){live.textContent='Upload '+s.uploadCount+' completed.';live.style.display='block';}prevStreaming=s.streamInProgress;prevUploads=s.uploadCount;}).catch(()=>{});};tick();setInterval(tick,1000);})();");
  page += F("</script></div></body></html>");
  return page;
}

// Serves the main page and clears one-shot feedback message after display.
void handleRoot() {
  server.send(200, "text/html", buildPage(pendingMessage));
  pendingMessage = String();
}

void handleStreamFile() {
  if (!fsMounted) {
    pendingMessage = F("Cannot stream: LittleFS is not mounted.");
    redirectToRoot();
    return;
  }

  if (streamInProgress || uploadInProgress) {
    pendingMessage = F("Cannot stream: another transfer is active.");
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

  if (!startPlaybackFromStaging()) {
    pendingMessage = F("Cannot stream: failed to start playback.");
    redirectToRoot();
    return;
  }

  persistUploadInfoToIni(F("manual"));
  pendingMessage = F("Started streaming selected file.");
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

  if (path == kIniFilePath) {
    pendingMessage = F("Cannot delete: esim.ini is protected.");
    redirectToRoot();
    return;
  }

  if (streamInProgress && path == currentFilePath) {
    pendingMessage = F("Cannot delete: file is currently streaming.");
    redirectToRoot();
    return;
  }

  if (!LittleFS.remove(path)) {
    pendingMessage = F("Delete failed.");
    redirectToRoot();
    return;
  }

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

void handleStatus() {
  String json;
  json.reserve(220);
  json += F("{");
  json += F("\"streamInProgress\":");
  json += (streamInProgress ? F("true") : F("false"));
  json += F(",\"streamOffset\":");
  json += String(streamOffset);
  json += F(",\"stagedFileBytes\":");
  json += String(stagedFileBytes);
  json += F(",\"uploadCount\":");
  json += String(uploadCount);
  json += F(",\"lastFileBytes\":");
  json += String(lastFileBytes);
  json += F(",\"totalBytesSent\":");
  json += String(totalBytesSent);
  json += F("}");

  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server.send(200, "application/json", json);
}

// Opens the staged file and starts background playback if conditions allow.
bool startPlaybackFromStaging(bool allowDuringUpload) {
  if (!fsMounted || streamInProgress || stagedFileBytes == 0 || currentFilePath.length() == 0) {
    return false;
  }

  if (!allowDuringUpload && uploadInProgress) {
    return false;
  }

  if (!LittleFS.exists(currentFilePath)) {
    return false;
  }

  if (playbackFile) {
    playbackFile.close();
  }

  playbackFile = LittleFS.open(currentFilePath, "r");
  if (!playbackFile) {
    return false;
  }

  streamOffset = 0;
  streamInProgress = true;
  return true;
}

// Handles serial upload command.
// Usage: 'u' + lenLo,lenHi + payload + cksLo,cksHi; ACK after len, each 128B, checksum.
void processSerialUploadCommand() {
  if (streamInProgress) {
    Serial.println(F("ERROR: playback is active."));
    return;
  }

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
  if (playbackFile) {
    playbackFile.close();
  }

  currentUploadRejected = false;
  currentUploadFsError = false;
  uploadInProgress = true;
  setUploadLed(true);
  const String requestedFilename = pendingSerialFilename;
  pendingSerialFilename = String();
  if (requestedFilename.length() > 0) {
    currentFilePath = makeWebUploadFilePath(requestedFilename);
    lastFilename = requestedFilename;
    ++nextUploadId;
  } else {
    currentFilePath = makeUploadFilePath(nextUploadId++);
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
  persistUploadInfoToIni(F("serial"));

  if (!startPlaybackFromStaging()) {
    currentUploadFsError = true;
    Serial.println(F("ERROR: failed to start playback."));
    return;
  }

  Serial.print(F("Serial upload OK: bytes="));
  Serial.println(stagedFileBytes);
  Serial.println(F("Ready."));
}

// Processes single-character serial commands (r, c, i, x, n, and u framed upload).
void processSerialCommands() {
  while (Serial.available() > 0) {
    const char cmd = static_cast<char>(Serial.read());
    Serial.println();
    if (cmd == 'c' || cmd == 'C') {
      if (streamInProgress) {
        playbackFile.close();
        streamInProgress = false;
         Serial.println(F("Serial command 'c': streaming canceled."));
      } else {
        Serial.println(F("Serial command 'c': no streaming in progress."));
      }
      clearDataBus();
    } else if (cmd == 'r' || cmd == 'R') {
      if (startPlaybackFromStaging()) {
        Serial.println(F("Replay started."));
      } else {
        Serial.println(F("Replay unavailable (busy or no staged file)."));
      }
    } else if (cmd == 'i' || cmd == 'I') {
      DBG_PRINTLN(F("Printing file debug info..."));
      printFileDebugInfo();
    } else if (cmd == 'x' || cmd == 'X') {
      printWebInfo();
    } else if (cmd == 'n' || cmd == 'N') {
      String receivedFilename;
      if (!readSerialFilename(receivedFilename)) {
        pendingSerialFilename = String();
        Serial.write(kSerialNakByte);
        Serial.println(F("ERROR: timeout while reading filename."));
      } else {
        pendingSerialFilename = receivedFilename;
        Serial.write(kSerialAckByte);
        DBG_PRINT(F("Serial filename accepted: "));
        DBG_PRINTLN(pendingSerialFilename);
      }
    } else if (cmd == 'u' || cmd == 'U') {
      processSerialUploadCommand();
    }
  }
}

// Handles multipart upload events and stores incoming data to staging file.
void handleUpload() {
  HTTPUpload &upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    setUploadLed(true);
    currentUploadRejected = streamInProgress;
    currentUploadFsError = false;

    DBG_PRINT(F("Upload start: "));
    DBG_PRINTLN(upload.filename);

    uploadInProgress = true;
    if (!currentUploadRejected) {
      currentUploadFsError = !fsMounted;
      const String formUploadName = server.hasArg("uploadName") ? server.arg("uploadName") : String();
      lastFilename = selectWebUploadName(upload.filename, formUploadName);
      ++nextUploadId;
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
    } else {
      DBG_PRINTLN(F("Upload start rejected: playback still in progress."));
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (!currentUploadRejected && !currentUploadFsError) {
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
    if (!currentUploadRejected && uploadStagingFile) {
      uploadStagingFile.close();
    }

    uploadInProgress = false;

    if (!currentUploadRejected && !currentUploadFsError && stagedFileBytes > 0) {
      if (!startPlaybackFromStaging(true)) {
        currentUploadFsError = true;
        DBG_PRINTLN(F("Upload end error: failed to start playback."));
      }
      lastFileBytes = stagedFileBytes;
      persistUploadInfoToIni(F("web"));
    } else if (!currentUploadRejected) {
      stagedFileBytes = 0;
      streamOffset = 0;
      streamInProgress = false;
      clearDataBus();
    }

    DBG_PRINT(F("Upload end: bytes="));
    DBG_PRINT(stagedFileBytes);
    DBG_PRINT(F(", fsError="));
    DBG_PRINT(currentUploadFsError ? F("true") : F("false"));
    DBG_PRINT(F(", streamInProgress="));
    DBG_PRINTLN(streamInProgress ? F("true") : F("false"));

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
    streamInProgress = false;
    clearDataBus();
    uploadInProgress = false;
    setUploadLed(false);
  }
}

// Finishes POST request and redirects with a short status message.
void handleUploadDone() {
  String message;
  if (currentUploadRejected) {
    message = F("Upload rejected: previous file is still streaming. Please wait until streaming is idle.");
  } else if (currentUploadFsError) {
    message = F("Upload failed: staging file write/read error.");
  } else if (stagedFileBytes == 0) {
    message = F("Upload received no data.");
  } else {
    message = F("Upload received. ");
    message += String(stagedFileBytes);
    message += F(" bytes processed from ");
    message += htmlEscape(lastFilename.length() ? lastFilename : String("unknown file"));
  }

  pendingMessage = message;
  redirectToRoot();
}

// Streams staged bytes in small batches to keep network and UI responsive.
void processPlaybackStream() {
  if (!streamInProgress || !playbackFile) {
    setUpSendLed(false);
    return;
  }

  setUpSendLed(true);
  size_t sentInSlice = 0;
  while (sentInSlice < kStreamBatchBytes && playbackFile.available()) {
    const int nextByte = playbackFile.read();
    if (nextByte < 0) {
      break;
    }

    outputByte(static_cast<uint8_t>(nextByte));
    ++streamOffset;
    ++totalBytesSent;
    ++sentInSlice;
  }

  if (!playbackFile.available()) {
    playbackFile.close();
    streamInProgress = false;
    ++uploadCount;
    clearDataBus();
  }

  delay(0);
}
}

// Initializes serial, SPI, filesystem, Wi-Fi, and web server routes.
void setup() {
  Serial.begin(115200);
  delay(100);

  SPI.begin();
  SPI.setFrequency(10000000);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);

  pinMode(kLatchPin, OUTPUT);
  digitalWrite(kLatchPin, HIGH);

  pinMode(kStrobePin, OUTPUT);
  digitalWrite(kStrobePin, HIGH); 

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LED_SENDDATA, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(LED_SENDDATA, LOW);
  Serial.println();
  clearDataBus();
  // Blink LED as a short delay to indicate boot and allow time for the serial monitor to connect.
  for (int i = 0; i < 10; ++i) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
  }

  if (!LittleFS.begin()) {
    Serial.println(F("ERROR: LittleFS init failed"));
    fsMounted = false;
  } else {
    fsMounted = true;
    loadStateFromIni();
    if (currentFilePath.length() > 0 && LittleFS.exists(currentFilePath)) {
      File restoreFile = LittleFS.open(currentFilePath, "r");
      if (restoreFile) {
        stagedFileBytes = restoreFile.size();
        lastFileBytes = stagedFileBytes;
        restoreFile.close();
      }
      if (stagedFileBytes > 0 && startPlaybackFromStaging()) {
        Serial.println(F("Startup: replaying last uploaded file."));
      }
    }
  }

#if defined(STA_MODE)
  WiFi.mode(WIFI_STA);
  WiFi.begin(kStaSsid, kStaPassword);
  Serial.print(F("Wi-Fi Connecting to SSID: "));
  Serial.println(kStaSsid);

  unsigned long startMs = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startMs) < 5000UL) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
    yield();
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiModeLabel = F("STA");
    wifiIpAddress = WiFi.localIP().toString();
  } else {
    wifiModeLabel = F("STA (fallback AP)");
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
  wifiModeLabel = F("AP");
  wifiIpAddress = WiFi.softAPIP().toString();
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/stream-file", HTTP_POST, handleStreamFile);
  server.on("/delete-file", HTTP_POST, handleDeleteFile);
  server.on("/upload", HTTP_POST, handleUploadDone, handleUpload);
  server.begin();
  printWebInfo();
}

// Main service loop for serial commands, HTTP handling, and playback.
void loop() {
  processSerialCommands();
  server.handleClient();
  processPlaybackStream();
}