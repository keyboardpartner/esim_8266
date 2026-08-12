#pragma once
// ################################################################################
//
//      #####  ####### ######  #     # ####### ######  
//     #     # #       #     # #     # #       #     # 
//     #       #       #     # #     # #       #     # 
//      #####  #####   ######  #     # #####   ######  
//           # #       #   #    #   #  #       #   #   
//     #     # #       #    #    # #   #       #    #  
//      #####  ####### #     #    #    ####### #     # 
//                                                     
// ################################################################################

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

  // Static web assets should keep a canonical filename and replace existing files.
  if (isNonStreamableFilePath(path)) {
    return path;
  }

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

bool isBitstreamFilePath(const String &path) {
  String filename = baseNameFromPath(path);
  filename.toLowerCase();
  return filename.endsWith(F(".bit"));
}

bool isNonStreamableFilePath(const String &path) {
  String filename = baseNameFromPath(path);
  filename.toLowerCase();
  return filename.endsWith(F(".css")) || filename.endsWith(F(".html")) || filename.endsWith(F(".htm"));
}

const __FlashStringHelper *primaryActionLabelForPath(const String &path) {
  if (isBitstreamFilePath(path)) {
    return F("Config");
  }
  if (isNonStreamableFilePath(path)) {
    return F("Invalid");
  }
  return F("Stream");
}

void warnIfLikelyFreshFilesystemImage() {
  if (!fsMounted) {
    likelyFreshFsImage = false;
    return;
  }

  Dir dir = LittleFS.openDir("/");
  size_t visibleFileCount = 0;
  size_t nonPackagedCount = 0;

  while (dir.next()) {
    const String path = normalizeFsPath(dir.fileName());

    if (path == kGlobalSettingsPath || path.endsWith(F(".ini"))) {
      continue;
    }

    ++visibleFileCount;
    if (path != F("/help.html") && path != F("/style.css")) {
      ++nonPackagedCount;
    }
  }

  likelyFreshFsImage = (visibleFileCount > 0 && nonPackagedCount == 0);

  if (likelyFreshFsImage) {
    Serial.println(F("WARN: LittleFS contains only packaged static files. If you ran uploadfs recently, user-uploaded files were replaced by the new image."));
  }
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

// ################################################################################
//
//      #####  ####### ####### ####### ### #     #  #####   #####  
//     #     # #          #       #     #  ##    # #     # #     # 
//     #       #          #       #     #  # #   # #       #       
//      #####  #####      #       #     #  #  #  # #  ####  #####  
//           # #          #       #     #  #   # # #     #       # 
//     #     # #          #       #     #  #    ## #     # #     # 
//      #####  #######    #       #    ### #     #  #####   #####  
//                                                                 
// ################################################################################
// Global settings file format:
// last_filename=<last uploaded filename>
// last_start=<last uploaded start address>
// startup_fpga_path=<path to FPGA bitstream file>
// sta_ssid=<Wi-Fi SSID for station mode>
// sta_password=<Wi-Fi password for station mode>
// ################################################################################

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
  iniFile.print(F("startup_fpga_path="));
  iniFile.println(startupFpgaPath);
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
    } else if (key == F("startup_fpga_path")) {
      const String candidatePath = normalizeFsPath(value);
      if (isValidFsPath(candidatePath) && isBitstreamFilePath(candidatePath)) {
        startupFpgaPath = candidatePath;
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

  if (!isValidFsPath(startupFpgaPath) || !isBitstreamFilePath(startupFpgaPath)) {
    startupFpgaPath = F("/fpga_main.bit");
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

String jsonEscape(const String &input) {
  String escaped;
  escaped.reserve(input.length() + 16);
  for (char c : input) {
    switch (c) {
      case '\\':
        escaped += F("\\\\");
        break;
      case '"':
        escaped += F("\\\"");
        break;
      case '\n':
        escaped += F("\\n");
        break;
      case '\r':
        escaped += F("\\r");
        break;
      case '\t':
        escaped += F("\\t");
        break;
      default:
        escaped += c;
        break;
    }
  }
  return escaped;
}


// ################################################################################
//
//     #######  #####        ######  ### ######  
//     #       #     #       #     #  #  #     # 
//     #       #             #     #  #  #     # 
//     #####    #####        #     #  #  ######  
//     #             #       #     #  #  #   #   
//     #       #     #       #     #  #  #    #  
//     #        #####        ######  ### #     # 
//                                                               
// ################################################################################
// For large directories, use sendFsDirectoryHtmlStreamed() 
// ################################################################################
                                       

// Streams the current LittleFS directory table to keep heap usage low.
void sendFsDirectoryHtmlStreamed() {
  if (!fsMounted) {
    server.sendContent(F("<p>LittleFS is not mounted.</p>"));
    return;
  }

  server.sendContent(F("<h3>Payload Files</h3>"));
  server.sendContent(F("<table style='width:100%;border-collapse:collapse'>"));
  server.sendContent(F("<thead><tr>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Name</th>"));
  server.sendContent(F("<th style='text-align:right;border-bottom:1px solid #334155;padding:6px 8px'>Size</th>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Start (dec/0x)</th>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Actions</th>"));
  server.sendContent(F("</tr></thead><tbody>"));

  Dir payloadDir = LittleFS.openDir("/");
  bool anyPayload = false;
  while (payloadDir.next()) {
    const String path = normalizeFsPath(payloadDir.fileName());
    if (path.endsWith(F(".ini")) || isNonStreamableFilePath(path)) {
      continue;
    }

    anyPayload = true;
    const size_t bytes = payloadDir.fileSize();
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
    row += F("'><button class='action-btn' type='submit'>");
    row += primaryActionLabelForPath(path);
    row += F("</button></form>");
    row += F("<form method='POST' action='/delete-file' onsubmit=\"return confirm('Delete file?');\">");
    row += F("<input type='hidden' name='path' value='");
    row += htmlEscape(path);
    row += F("'><button class='action-btn' type='submit'>Delete</button></form>");
    row += F("</div></td></tr>");
    server.sendContent(row);
    delay(0); yield();
  }

  if (!anyPayload) {
    server.sendContent(F("<tr><td colspan='4' style='padding:8px;color:#94a3b8'>No payload files present.</td></tr>"));
  }
  server.sendContent(F("</tbody></table>"));

  server.sendContent(F("<h3 style='margin-top:16px'>System Files</h3>"));
  server.sendContent(F("<table style='width:100%;border-collapse:collapse'>"));
  server.sendContent(F("<thead><tr>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Name</th>"));
  server.sendContent(F("<th style='text-align:right;border-bottom:1px solid #334155;padding:6px 8px'>Size</th>"));
  server.sendContent(F("<th style='text-align:left;border-bottom:1px solid #334155;padding:6px 8px'>Actions</th>"));
  server.sendContent(F("</tr></thead><tbody>"));

  Dir systemDir = LittleFS.openDir("/");
  bool anySystem = false;
  while (systemDir.next()) {
    const String path = normalizeFsPath(systemDir.fileName());
    if (path.endsWith(F(".ini")) || !isNonStreamableFilePath(path)) {
      continue;
    }

    anySystem = true;
    const size_t bytes = systemDir.fileSize();
    const String displayName = baseNameFromPath(path);
    const String displayText = displayName.length() ? displayName : path;

    String row;
    row.reserve(700);
    row += F("<tr>");
    row += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937'><a href='/download-file?path=");
    row += urlEncodeComponent(path);
    row += F("' style='color:#93c5fd;text-decoration:none'><code>");
    row += htmlEscape(displayText);
    row += F("</code></a></td>");
    row += F("<td style='padding:6px 8px;text-align:right;border-bottom:1px solid #1f2937'>");
    row += String(bytes);
    row += F(" bytes</td>");
    row += F("<td style='padding:6px 8px;border-bottom:1px solid #1f2937;white-space:nowrap;vertical-align:middle'>");
    row += F("<div class='actions'>");
    row += F("<form method='POST' action='/delete-file' onsubmit=\"return confirm('Delete file?');\">");
    row += F("<input type='hidden' name='path' value='");
    row += htmlEscape(path);
    row += F("'><button class='action-btn' type='submit'>Delete</button></form>");
    row += F("</div></td></tr>");
    server.sendContent(row);
    delay(0); yield();
  }

  if (!anySystem) {
    server.sendContent(F("<tr><td colspan='3' style='padding:8px;color:#94a3b8'>No system files present.</td></tr>"));
  }

  server.sendContent(F("</tbody></table>"));
}


// ################################################################################
//
//     ######  ####### ####### ####### 
//     #     # #     # #     #    #    
//     #     # #     # #     #    #    
//     ######  #     # #     #    #    
//     #   #   #     # #     #    #    
//     #    #  #     # #     #    #    
//     #     # ####### #######    #    
//                                     
// ################################################################################
// Streams the uploader page in small chunks to avoid large temporary String allocations.
// ################################################################################

void handleRoot() {
  const String message = pendingMessage;
  pendingMessage = String();

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");

  server.sendContent(F("<!doctype html><html><head><meta charset='utf-8'>"));
  server.sendContent(F("<meta name='viewport' content='width=device-width,initial-scale=1'>"));
  #ifdef GODIL_SPI
    server.sendContent(F("<title>GODIL Binary Uploader</title>"));
  #endif
  #ifdef ESIM_SPI
    server.sendContent(F("<title>ESIM Binary Uploader</title>"));
  #endif
  #ifdef PEPS_SPI
    server.sendContent(F("<title>PEPS Binary Uploader</title>"));
  #endif
  #ifdef JTAG_SPARTAN6
    server.sendContent(F("<title>FPGA Configurator and Uploader</title>"));
  #endif
  server.sendContent(F("<link rel='stylesheet' href='/style.css'></head><body><div class='card'>"));

  #ifdef GODIL_SPI
    server.sendContent(F("<h1>GODIL Binary Uploader</h1><h3>by KeyboardPartner 7/2026</h3>"));
  #endif
  #ifdef ESIM_SPI
    server.sendContent(F("<h1>ESIM Binary Uploader</h1><h3>by KeyboardPartner 7/2026</h3>"));
  #endif
  #ifdef PEPS_SPI
    server.sendContent(F("<h1>PEPS Binary Uploader</h1><h3>by KeyboardPartner 7/2026</h3>"));
  #endif
  #ifdef JTAG_SPARTAN6
    server.sendContent(F("<h1>XC6SLX9 FPGA JTAG Configurator and Uploader</h1><h3>by KeyboardPartner 7/2026</h3>"));
  #endif

  server.sendContent(F("<form id='uploadForm' method='POST' action='/upload' enctype='multipart/form-data'>"));
  server.sendContent(F("<table style='width:100%;border-collapse:separate;border-spacing:8px 6px;margin:0'><tr>"));
  server.sendContent(F("<td style='padding:0;width:78%'><label for='binfile' style='display:block;margin:0 0 6px'>Binary file</label><input id='binfile' name='binfile' type='file' accept='.bin,.bit,.rom,application/octet-stream' required></td>"));
  server.sendContent(F("<td style='padding:0;width:22%'><label for='uploadStart' style='display:block;margin:0 0 6px'>Start (dec/0x)</label><input id='uploadStart' name='start' type='text' value='"));
  server.sendContent(htmlEscape(formatAddressForInput(lastUploadStartAddr)));
  server.sendContent(F("' maxlength='10' style='max-width:11ch' required></td>"));
  server.sendContent(F("</tr></table>"));
  server.sendContent(F("<input id='uploadName' name='uploadName' type='hidden' value=''>"));
  server.sendContent(F("</form>"));
  server.sendContent(F("<div class='upload-actions-row' style='display:flex;gap:8px;align-items:center;flex-wrap:wrap'>"));
  server.sendContent(F("<button id='uploadButton' type='submit' form='uploadForm'>Upload and stream</button>"));
  server.sendContent(F("<form method='POST' action='/erase-eprom' style='margin:0' onsubmit=\"return confirm('Erase Device Memory now?');\">"));
  server.sendContent(F("<button type='submit' style='background:#ef4444;color:#ffffff'>Erase Device Memory</button>"));
  server.sendContent(F("</form>"));
  server.sendContent(F("<form method='GET' action='/settings.html' style='margin:0'>"));
  server.sendContent(F("<button type='submit' style='background:#f59e0b;color:#111827'>Edit defaults</button>"));
  server.sendContent(F("</form>"));
  server.sendContent(F("<form method='GET' action='/help.html' style='margin:0'>"));
  server.sendContent(F("<button type='submit'>Help</button>"));
  server.sendContent(F("</form>"));
  server.sendContent(F("</div>"));

  #if defined(GODIL_SPI) || defined(JTAG_SPARTAN6) 
    server.sendContent(F("<h3>Dump EPROM</h3>"));
    server.sendContent(F("<form method='POST' action='/dump-eprom'>"));
    server.sendContent(F("<table style='width:100%;border-collapse:separate;border-spacing:8px 6px;margin:0'><tr>"));
    server.sendContent(F("<td style='padding:0;width:50%'><label for='dumpFilename' style='display:block;margin:0 0 6px'>Filename</label><input id='dumpFilename' name='filename' type='text' value='eprom_dump.bin' required></td>"));
    server.sendContent(F("<td style='padding:0;width:28%'><label for='dumpStart' style='display:block;margin:0 0 6px'>Start (dec/0x)</label><input id='dumpStart' name='start' type='text' value='0x0000' required></td>"));
    server.sendContent(F("<td style='padding:0;width:22%'><label for='dumpLen' style='display:block;margin:0 0 6px'>Length</label><input id='dumpLen' name='len' type='text' value='4096' required></td>"));
    server.sendContent(F("</tr></table>"));
    server.sendContent(F("<button type='submit'>Dump EPROM</button></form>"));
  #endif

  size_t fsTotalBytes = 0;
  size_t fsUsedBytes = 0;
  size_t fsFreeBytes = 0;
  size_t fsTotalKiB = 0;
  size_t fsUsedKiB = 0;
  size_t fsFreeKiB = 0;
  if (fsMounted) {
    FSInfo fsInfo;
    if (LittleFS.info(fsInfo)) {
      fsTotalBytes = fsInfo.totalBytes;
      fsUsedBytes = fsInfo.usedBytes;
      fsFreeBytes = (fsTotalBytes >= fsUsedBytes) ? (fsTotalBytes - fsUsedBytes) : 0;
      fsTotalKiB = fsTotalBytes / 1024;
      fsUsedKiB = fsUsedBytes / 1024;
      fsFreeKiB = fsFreeBytes / 1024;
    }
  }

  server.sendContent(F("<h2>Status</h2><ul class='status-list'>"));
  server.sendContent(F("<li>Wi-Fi Mode: <code>"));
  server.sendContent(htmlEscape(wifiModeLabel));
  server.sendContent(F("</code> IP Address: <code>"));
  server.sendContent(htmlEscape(wifiIpAddress));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>AP SSID: <code>"));
  server.sendContent(htmlEscape(String(kApSsid)));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>Last file: <code>"));
  server.sendContent(htmlEscape(lastFilename.length() ? lastFilename : String("none")));
  server.sendContent(F("</code> Size: <code>"));
  server.sendContent(String(lastFileBytes));
  server.sendContent(F("</code> Start: <code>"));
  server.sendContent(formatAddressForInput(lastUploadStartAddr));
  server.sendContent(F("</code></li>"));
  server.sendContent(F("<li>File System: <code id='stFsUsed'>"));
  server.sendContent(String(fsUsedKiB));
  server.sendContent(F("</code> KiB used, <code id='stFsFree'>"));
  server.sendContent(String(fsFreeKiB));
  server.sendContent(F("</code> KiB free, <code id='stFsTotal'>"));
  server.sendContent(String(fsTotalKiB));
  server.sendContent(F("</code> KiB total</li>"));
  #ifdef JTAG_SPARTAN6
    server.sendContent(F("<li>FPGA Configuration: <code>"));
    server.sendContent(startupFpgaPath);
    server.sendContent(F("</code></li>"));
    server.sendContent(F("<li>FPGA ID code: <code>"));
    server.sendContent(formatAddressForInput(jtagIDcode));
    server.sendContent(F("</code> Version: <code>"));
    server.sendContent(formatAddressForInput(fpgaVersion));
    server.sendContent(F("</code></li>"));
  #endif
  server.sendContent(F("</ul>"));

  server.sendContent(F("<div class='messages'>"));
  if (message.length() > 0) {
    server.sendContent(F("<div class='msg'>"));
    server.sendContent(htmlEscape(message));
    server.sendContent(F("</div>"));
  }
  if (likelyFreshFsImage) {
    server.sendContent(F("<div class='msg wait'>LittleFS contains only packaged static files. If you recently used uploadfs, previously uploaded runtime files were replaced by the filesystem image.</div>"));
  }
  server.sendContent(F("<div id='waitNotice' class='msg wait' style='display:none'>Upload in progress. Please wait until the page reports completion.</div>"));
  server.sendContent(F("<div id='liveNotice' class='msg ok' style='display:none'></div>"));
  server.sendContent(F("</div>"));

  sendFsDirectoryHtmlStreamed();

  server.sendContent(F("<script>"));
  server.sendContent(F("(()=>{const form=document.getElementById('uploadForm');const btn=document.getElementById('uploadButton');const wait=document.getElementById('waitNotice');const live=document.getElementById('liveNotice');const fileInput=document.getElementById('binfile');const uploadName=document.getElementById('uploadName');if(!form||!btn||!wait||!live){return;}form.addEventListener('submit',()=>{if(uploadName&&fileInput&&fileInput.files&&fileInput.files[0]){uploadName.value=fileInput.files[0].name||'';}btn.disabled=true;btn.textContent='Uploading...';wait.style.display='block';});const tick=()=>{fetch('/status',{cache:'no-store'}).then(r=>r.json()).then(s=>{const stLast=document.getElementById('stLastFileBytes');const stProgress=document.getElementById('stProgress');const stTotal=document.getElementById('stTotalBytes');const stFsUsed=document.getElementById('stFsUsed');const stFsFree=document.getElementById('stFsFree');const stFsTotal=document.getElementById('stFsTotal');if(stLast){stLast.textContent=s.lastFileBytes;}if(stProgress){stProgress.textContent=s.streamOffset+' / '+s.stagedFileBytes;}if(stTotal){stTotal.textContent=s.totalBytesSent;}if(stFsUsed){stFsUsed.textContent=Math.floor((s.fsUsedBytes||0)/1024);}if(stFsFree){stFsFree.textContent=Math.floor((s.fsFreeBytes||0)/1024);}if(stFsTotal){stFsTotal.textContent=Math.floor((s.fsTotalBytes||0)/1024);}}).catch(()=>{});};tick();setInterval(tick,1000);})();"));
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

  if (isNonStreamableFilePath(path)) {
    pendingMessage = F("Cannot stream: .css/.html files are not streamable.");
    redirectToRoot();
    return;
  }

  const bool stagedIsBitstream = isBitstreamFilePath(path);
  if (!startPlaybackFromStaging(startAddr)) {
    pendingMessage = stagedIsBitstream ? F("Cannot config: failed to process file.")
                                       : F("Cannot stream: failed to send file.");
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

  pendingMessage = stagedIsBitstream ? F("FPGA configured.") : F("Selected file streamed to device.");
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

  lastFilename = String();
  lastUploadStartAddr = 0;
  startupFpgaPath = F("/fpga_main.bit");
  webUploadStartAddr = 0;
  staSsid = String(kStaSsid);
  staPassword = String(kStaPassword);
  pendingMessage = F("Saved defaults cleared.");
  redirectToRoot();
}

void handleSettingsData() {
  String json;
  json.reserve(320);
  json += F("{");
  json += F("\"last_filename\":\"");
  json += jsonEscape(lastFilename);
  json += F("\",");
  json += F("\"last_start\":\"");
  json += jsonEscape(formatAddressForInput(lastUploadStartAddr));
  json += F("\",");
  json += F("\"startup_fpga_path\":\"");
  json += jsonEscape(startupFpgaPath);
  json += F("\",");
  json += F("\"sta_ssid\":\"");
  json += jsonEscape(staSsid);
  json += F("\",");
  json += F("\"sta_password\":\"");
  json += jsonEscape(staPassword);
  json += F("\"");
  json += F("}");

  server.sendHeader("Cache-Control", "no-store, no-cache, must-revalidate");
  server.send(200, "application/json", json);
}

void handleSaveSettings() {
  if (!fsMounted) {
    server.sendHeader("Location", "/settings.html?err=LittleFS%20is%20not%20mounted", true);
    server.send(303, "text/plain", "");
    return;
  }

  String requestedLastFilename = server.hasArg("last_filename") ? server.arg("last_filename") : String();
  String requestedLastStart = server.hasArg("last_start") ? server.arg("last_start") : String();
  String requestedStartupPath = server.hasArg("startup_fpga_path") ? server.arg("startup_fpga_path") : String();
  String requestedStaSsid = server.hasArg("sta_ssid") ? server.arg("sta_ssid") : String();
  String requestedStaPassword = server.hasArg("sta_password") ? server.arg("sta_password") : String();

  requestedStaSsid.trim();
  requestedStaPassword.trim();
  requestedStartupPath = normalizeFsPath(requestedStartupPath);

  uint32_t parsedStart = 0;
  if (!parseUnsignedValue(requestedLastStart, parsedStart)) {
    server.sendHeader("Location", "/settings.html?err=Invalid%20last_start%20value", true);
    server.send(303, "text/plain", "");
    return;
  }

  if (!isValidFsPath(requestedStartupPath) || !isBitstreamFilePath(requestedStartupPath)) {
    server.sendHeader("Location", "/settings.html?err=Invalid%20startup_fpga_path%20value", true);
    server.send(303, "text/plain", "");
    return;
  }

  const size_t maxNameLen = (kMaxFsPathLength > 1) ? (kMaxFsPathLength - 1) : 1;
  lastFilename = fitFilenameToLength(sanitizeUploadFilename(requestedLastFilename), maxNameLen);
  lastUploadStartAddr = parsedStart;
  startupFpgaPath = requestedStartupPath;
  staSsid = requestedStaSsid;
  staPassword = requestedStaPassword;

  if (!saveGlobalSettings()) {
    server.sendHeader("Location", "/settings.html?err=Failed%20to%20save%20.settings.ini", true);
    server.send(303, "text/plain", "");
    return;
  }

  server.sendHeader("Location", "/settings.html?saved=1", true);
  server.send(303, "text/plain", "");
}

void handleEraseEprom() {
  if (uploadInProgress) {
    pendingMessage = F("Cannot erase: upload is active.");
    redirectToRoot();
    return;
  }

  eraseEPROM();
  pendingMessage = F("EPROM erased.");
  redirectToRoot();
}

void handleDumpEprom() {
#if defined(GODIL_SPI) || defined(JTAG_SPARTAN6)
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
  pendingMessage = F("EPROM dump is only available in FPGA mode.");
  redirectToRoot();
#endif
}

void handleStatus() {
  size_t fsTotalBytes = 0;
  size_t fsUsedBytes = 0;
  size_t fsFreeBytes = 0;
  if (fsMounted) {
    FSInfo fsInfo;
    if (LittleFS.info(fsInfo)) {
      fsTotalBytes = fsInfo.totalBytes;
      fsUsedBytes = fsInfo.usedBytes;
      fsFreeBytes = (fsTotalBytes >= fsUsedBytes) ? (fsTotalBytes - fsUsedBytes) : 0;
    }
  }

  String json;
  json.reserve(320);
  json += F("{");
  json += F("\"streamOffset\":");
  json += String(streamOffset);
  json += F(",\"stagedFileBytes\":");
  json += String(stagedFileBytes);
  json += F(",\"lastFileBytes\":");
  json += String(lastFileBytes);
  json += F(",\"totalBytesSent\":");
  json += String(totalBytesSent);
  json += F(",\"fsTotalBytes\":");
  json += String(fsTotalBytes);
  json += F(",\"fsUsedBytes\":");
  json += String(fsUsedBytes);
  json += F(",\"fsFreeBytes\":");
  json += String(fsFreeBytes);
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
    set_static_message(F("upl"));
    currentUploadFsError = false;

    DPRINT(F("Upload start: "));
    DPRINTLN(upload.filename);

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
        DPRINTLN(F("Upload start error: failed to open staging file."));
      }
    } else {
      DPRINTLN(F("Upload start error: filesystem not mounted."));
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (!currentUploadFsError) {
      const size_t written = uploadStagingFile.write(upload.buf, upload.currentSize);
      stagedFileBytes += written;
      DPRINT(F("Upload write chunk="));
      DPRINT(upload.currentSize);
      DPRINT(F(", written="));
      DPRINT(written);
      DPRINT(F(", total="));
      DPRINTLN(stagedFileBytes);
      if (written != upload.currentSize) {
        currentUploadFsError = true;
        DPRINTLN(F("Upload write error: short write to staging file."));
      }

      // Keep the HTTP/TCP stack responsive while receiving upload chunks.
      delay(0); yield();
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

    DPRINT(F("Upload end: bytes="));
    DPRINT(stagedFileBytes);
    DPRINT(F(", fsError="));
    DPRINT(currentUploadFsError ? F("true") : F("false"));
    DPRINTLN();

    setUploadLed(false);
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    DPRINTLN(F("Upload aborted."));
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
    if (isBitstreamFilePath(currentFilePath)) {
      startupFpgaPath = normalizeFsPath(currentFilePath);
    }
    if (!server.hasArg("start") || !parseUnsignedValue(server.arg("start"), webUploadStartAddr)) {
      currentUploadStartArgInvalid = true;
      message = F("Upload failed: invalid start address.");
    } else if (!saveStartAddressForFile(currentFilePath, webUploadStartAddr)) {
      message = F("Upload failed: could not save start address.");
    } else {
      if (isNonStreamableFilePath(currentFilePath)) {
        message = F("Upload stored. Streaming skipped for .css/.html file types.");
      } else if (!startPlaybackFromStaging(webUploadStartAddr)) {
        message = isBitstreamFilePath(currentFilePath)
                      ? F("Upload failed: could not configure staged file.")
                      : F("Upload failed: could not stream staged file.");
      }

      if (message.length() == 0) {
        lastFilename = baseNameFromPath(currentFilePath);
        lastUploadStartAddr = webUploadStartAddr;
        if (isBitstreamFilePath(currentFilePath)) {
          startupFpgaPath = normalizeFsPath(currentFilePath);
        }
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
  }
  set_rdy_message();

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
    currentWifiMode = WifiMode_STA;
  } else {
    wifiModeLabel = F("AP (fallback from STA)");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(kApSsid, kApPassword);
    wifiIpAddress = WiFi.softAPIP().toString();
    currentWifiMode = WifiMode_AP;
    // Blink LED to indicate AP mode fallback due to STA connection failure.
    for (int i = 0; i < 10; ++i) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    #ifndef JTAG_SPARTAN6
      digitalWrite(LED_SENDDATA, LOW);
    #endif
  }
#else
  Serial.print(F("Wi-Fi in AP mode"));
  Serial.println(kApSsid);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(kApSsid, kApPassword);
  wifiModeLabel = F("AP ");
  wifiIpAddress = WiFi.softAPIP().toString();
  currentWifiMode = WifiMode_AP;
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
#endif

  server.on("/", HTTP_GET, handleRoot);
  server.serveStatic("/style.css", LittleFS, "/style.css", "max-age=300");
  server.serveStatic("/help.html", LittleFS, "/help.html", "max-age=300");
  server.serveStatic("/settings.html", LittleFS, "/settings.html", "max-age=300");
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/settings-data", HTTP_GET, handleSettingsData);
  server.on("/save-settings", HTTP_POST, handleSaveSettings);
  server.on("/download-file", HTTP_GET, handleDownloadFile);
  server.on("/stream-file", HTTP_POST, handleStreamFile);
  server.on("/delete-file", HTTP_POST, handleDeleteFile);
  server.on("/reset-settings", HTTP_POST, handleResetSettings);
  server.on("/erase-eprom", HTTP_POST, handleEraseEprom);
  server.on("/dump-eprom", HTTP_POST, handleDumpEprom);
  server.on("/upload", HTTP_POST, handleUploadDone, handleUpload);
  server.begin();
}
