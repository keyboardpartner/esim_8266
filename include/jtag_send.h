#ifndef JTAG_SEND_H
#define JTAG_SEND_H

// ##############################################################################
//
//           # #######    #     #####      #####  ####### #     # ######  
//           #    #      # #   #     #    #     # #       ##    # #     # 
//           #    #     #   #  #          #       #       # #   # #     # 
//           #    #    #     # #  ####     #####  #####   #  #  # #     # 
//     #     #    #    ####### #     #          # #       #   # # #     # 
//     #     #    #    #     # #     #    #     # #       #    ## #     # 
//      #####     #    #     #  #####      #####  ####### #     # ######  
//                                                                        
// ##############################################################################
// SEND JTAG CONFIGURATION FROM LITTLEFS
// ##############################################################################

// Optimized for speed on ESP8266 using direct GPIO register access and faster loops 
// by Carsten Meyer 7/2026, info@keyboardpartner.de
// Brought down configuration time for a 334 KByte bitstream from 3.4 seconds to 921 ms (!) on ESP8266

// Note: It might be possible to speed up even more using the SPI for shifting out the data
// stream bytes. However, behaviour of the SPI pins on ESP8266 on begin()/end() is not determined
// and might be unstable, so we stick to bit-banging for now.

// Note that the pins used here for bit banging are easy accessible on the ESP8266.
// GPIO16 (User/D0) does not map to the standard GPOS/GPOC registers and cannot use this method.

// Based on a sketch by RSP @ Embedded Systems Lab (ESL), KMUTNB, Bangkok / Thailand
// Date: 2017-07-06
// Objective: This sketch shows how to use an ESP8266 module to
//   configure the Xilinx Spartan-6 FPGA device using the JTAG port.
//   The bitstream file ("TOP.BIN") and its associated MD5 checksum file ("TOP.MD5")
//   are stored in a microSD attached to the ESP8266 via the SPI bus.
//   The MD5 checksum calculation is performed first before loading the bitstream.
//   This sketch can successfully load the bitstream into the Xilinx Spartan 6SLX9
//   FPGA device on the Mojo v3 board.

#include <Arduino.h>
#include <LittleFS.h>
#include <cstdlib>
#include <cstdio>
#include "global_vars.h" // for some #defines

// ESP8266 Pins for JTAG: 
const int TCK_PIN = 5; // D1 / GPIO-5 (output)
const int TDO_PIN = 4; // D2 / GPIO-4 (input)
const int TDI_PIN = 0; // D3 / GPIO-0 (output)
const int TMS_PIN = 2; // D4 / GPIO-2 (output)

#define MAX_BUF_SIZE (4096)

// union of 8 and 32 bit buffer for faster access -- does not pay due to wrong byte order
// union {
//   uint8_t bytes[MAX_BUF_SIZE];
//   uint32_t words[MAX_BUF_SIZE / 4];
// } buffer;

uint8_t buf[MAX_BUF_SIZE];

uint32_t jtagIDcode = 0;
uint32_t fpgaVersion = 0;
bool fpgaValid = false;
bool fpgaConfigured = false;

// see: Spartan-6 FPGA Configuration User Guide UG380 (v2.10) March 31, 2017
#define XILINX_IR_LEN             (6)
#define XILINX_USER1_INSTR        (0x02)  // 000010
#define XILINX_USER2_INSTR        (0x03)  // 000011
#define XILINX_USER3_INSTR        (0x1A)  // 011010
#define XILINX_USER4_INSTR        (0x1B)  // 011011
#define XILINX_CFG_OUT_INSTR      (0x04)  // 000100
#define XILINX_CFG_IN_INSTR       (0x05)  // 000101
#define XILINX_BYPASS_INSTR       (0x3F)  // 111111
#define XILINX_IDCODE_INSTR       (0x09)  // 001001
#define XILINX_USERCODE_INSTR     (0x08)  // 001000
#define XILINX_JPROGRAM_INSTR     (0x0B)  // 001011
#define XILINX_JSTART_INSTR       (0x0C)  // 001100
#define XILINX_JSHUTDOWN_INSTR    (0x0D)  // 001101 
#define XILINX_S6LX9_IDCODE       (0x04001093)  // Spartan-6 LX9 FPGA device ID code
#define XILINX_3S400_IDCODE       (0x0141C093)  // Spartan-3 S400 FPGA device ID code


// ##############################################################################
//
//     #     # ####### #       ######  ####### ######   #####  
//     #     # #       #       #     # #       #     # #     # 
//     #     # #       #       #     # #       #     # #       
//     ####### #####   #       ######  #####   ######   #####  
//     #     # #       #       #       #       #   #         # 
//     #     # #       #       #       #       #    #  #     # 
//     #     # ####### ####### #       ####### #     #  #####  
//                                                             
// ##############################################################################
// Optimized for speed on ESP8266 using direct GPIO register access
// ##############################################################################

// inline codes for bit banging the JTAG signals on ESP8266
#define TMS_HIGH  GPOS = (1<<TMS_PIN)
#define TMS_LOW   GPOC = (1<<TMS_PIN)
#define TCK_HIGH  GPOS = (1<<TCK_PIN)
#define TCK_LOW   GPOC = (1<<TCK_PIN)
#define TDI_HIGH  GPOS = (1<<TDI_PIN)
#define TDI_LOW   GPOC = (1<<TDI_PIN)

#define TCK_PULSE  GPOC = (1<<TCK_PIN); GPOS = (1<<TCK_PIN)

void jtag_tl_reset() {
  TMS_HIGH;
  for ( int i=0; i < 5; i++ ) {
    TCK_PULSE;
  }
}

void jtag_goto_runtest_idle() {
  jtag_tl_reset();
  TMS_LOW;
  TCK_PULSE;  // goto Run-Test/Idle
}

void jtag_load_ir(uint32_t instr) {
  // start from Run-Test/Idle state
  jtag_goto_runtest_idle();
  TMS_HIGH;
  TCK_PULSE;  // goto Select-DR-Scan
  TCK_PULSE;  // goto Select-IR-Scan
  TMS_LOW;
  TCK_PULSE;  // goto Capture-IR
  TCK_PULSE;  // goto shift-IR
  for ( int i=0; i < XILINX_IR_LEN; i++ ) {
    // shift LSB first
    if (instr & 1) {
       TDI_HIGH;
     } else {
       TDI_LOW;
     }
     // leave with TMS high on last bit to go to Exit1-IR state
     if (i==(XILINX_IR_LEN-1)) {
       TMS_HIGH;
     }
     TCK_PULSE; // goto Exit1-IR or shift-DR
     instr = (instr >> 1);
  }
  // TMS is still high, so we are now in Exit1-IR state
  TCK_PULSE;  // goto Update-IR
  TMS_LOW;
  TCK_PULSE;  // back Run-Test/Idle
}

void jtag_shift_last_byte(uint8_t data) {
  // shift the last byte with TMS high on last bit
  for ( int i=0; i < 8; i++ ) {           // for each bit of the byte data
    if (i==7) {  // check for the last bit
      TMS_HIGH;  // goto Exit1-DR on last bit
    }     
    if (data & 0x80) {
      TDI_HIGH;
    } else {
      TDI_LOW;
    }
    data = data << 1;                     // shift-to-left
    TCK_PULSE;
  }
}

void jtag_shift_chunk(int chunk_size) {
  // shift all bytes of the chunk with TMS low
  for ( int j=0; j < chunk_size; j++) {    // for each 32 bit word of the chunk data
    uint8_t data = buf[j];
    uint8_t bit_state_old = ~(data & 0x80);
    for ( int i=0; i < 8; i++ ) {           // for each bit of the byte data
      uint8_t bit_state = (data & 0x80);
      if (bit_state != bit_state_old) {
        // set TDI only if the bit state has changed, otherwise we can skip it for speed
        if (bit_state) {
          TDI_HIGH;
        } else {
          TDI_LOW;
        }
      }
      bit_state_old = bit_state;
      data = data << 1;                     // shift-to-left
      TCK_PULSE;
    }
  }
}

void jtag_shift_last_chunk(int chunk_size) {
  // jtag_shift_chunk(chunk_size);
  jtag_shift_chunk(chunk_size - 1); // shift all but the last byte
  // shift the last byte with TMS high on last bit
  jtag_shift_last_byte(buf[chunk_size-1]);
  jtag_shift_last_byte(0);
}


uint32_t jtag_get_IDcode() {
  TCK_LOW;
  pinMode( TCK_PIN, OUTPUT );
  pinMode( TMS_PIN, OUTPUT );
  pinMode( TDI_PIN, OUTPUT );
  pinMode( TDO_PIN, INPUT );
  int tdo;
  uint32_t idcode = 0;
  // start from Run-Test/Idle state
  jtag_load_ir( XILINX_IDCODE_INSTR );
  TMS_HIGH;
  TCK_PULSE;  // goto Select-DR-Scan
  TMS_LOW;
  TCK_PULSE;  // goto Capture-DR
  TCK_PULSE;  // goto Shift-DR
  // now in Shift-DR state
  TDI_LOW;
  TMS_LOW;
  for ( int i=0; i < 32; i++ ) {
    if (i==31) {
      TMS_HIGH;  // goto Exit1-DR on last bit
    }
    TCK_PULSE;
    tdo = (GPI >> TDO_PIN) & 1;
    idcode = (tdo << 31) | (idcode >> 1); // shift MSB first
  }
  // now in Exit1-DR state
  TMS_HIGH;
  TCK_PULSE;  // goto Update-DR
  TMS_LOW;
  TCK_PULSE;  // goto Run-Test/Idle
  return idcode;
}

int jtag_shift_file(File bitfile) {
  int num_read_total = 0;
  int file_size = bitfile.size();  // get file size (in bytes)
  // start from Run-Test/Idle state
  TMS_HIGH; 
  TCK_PULSE; // goto Select-DR-Scan
  TMS_LOW;
  TCK_PULSE; // goto Capture-DR
  TCK_PULSE; // goto Shift-DR

  // now in Shift-DR state, TMS is still low, so we can shift in the data stream
  int chunk_count = 0;
  Serial.print( "Progress: " );

  while ( bitfile.available() ) {
    int chunk_size = bitfile.read(buf, MAX_BUF_SIZE-1);
    num_read_total += chunk_size;
    if (num_read_total==file_size) {
      // last chunk, so we need to go to Exit1-DR state after the last bit
      jtag_shift_last_chunk(chunk_size);
    } else {
      jtag_shift_chunk(chunk_size);
    }
    if (chunk_count % 16 == 0) {
      delay(0); yield();
      Serial.print(".");
    }
    chunk_count++;
  }
  Serial.println(F(" Done."));
  
  // now in Exit1-DR state
  TMS_HIGH;
  TCK_PULSE;  // goto Update-DR
  TMS_LOW;
  TCK_PULSE;  // goto Run-Test/Idle
  return num_read_total;
}

void jtag_startup_clk() {
  // TMS still low, toggle TCK for startup sequence
  for ( int i=0; i < 31; i++ ) {
    TCK_PULSE;  // stay at Run-Test/Idle state
  }
  TMS_HIGH;
  TCK_PULSE;
  TCK_PULSE;
  TCK_PULSE;
}

// ############################################################################
//
//      #####  ####### #     # ####### ###  #####  
//     #     # #     # ##    # #        #  #     # 
//     #       #     # # #   # #        #  #       
//     #       #     # #  #  # #####    #  #  #### 
//     #       #     # #   # # #        #  #     # 
//     #     # #     # #    ## #        #  #     # 
//      #####  ####### #     # #       ###  #####  
//
// ############################################################################
// Configure FPGA using JTAG port and the bitstream file stored in LittleFS
// ############################################################################


bool jtagCheckIDcode() {
  jtagIDcode = jtag_get_IDcode();
  if ((jtagIDcode & 0x0FFF) == 0x0093) {
    // Valid Xilinx Manufacturer ID code
    fpgaValid = true;
    return true;
  } else {
    fpgaConfigured = false;
    fpgaValid = false;
    return false;
  }
}

void jtagConfigure(const String &config_file) {
  TCK_LOW;
  pinMode( TCK_PIN, OUTPUT );
  pinMode( TMS_PIN, OUTPUT );
  pinMode( TDI_PIN, OUTPUT );
  pinMode( TDO_PIN, INPUT );
  Serial.println();
  printCenteredSerial(F("FPGA Config"));
  // configure GPIO pins for JTAG link 
  int num_read_total = 0;
  String str;
  
  File f = LittleFS.open(config_file.c_str(), "r");
  if (!f) {
    Serial.print(F("Cannot open bitstream file: "));
    Serial.println(config_file);
    return;
  }

  // goto RunTest/Idle
  Serial.print(F("Sending File: "));
  Serial.println(config_file);
 uint32_t ts = millis(); // Stopwatch for measuring configuration time

  // refer to page 171 UG380 Spartan-6 FPGA Configuration User Guide and sample SVF file

  // aus SVF file entnommen
  jtag_load_ir( XILINX_BYPASS_INSTR );
  jtag_load_ir( XILINX_JPROGRAM_INSTR );
  jtag_load_ir( XILINX_CFG_IN_INSTR );
  delay(10); // wait for 10 ms to allow the FPGA to enter configuration mode
  jtag_load_ir( XILINX_CFG_IN_INSTR );
  num_read_total = jtag_shift_file(f);
  
  jtag_load_ir( XILINX_JSTART_INSTR );
  jtag_startup_clk();
  jtag_load_ir( XILINX_BYPASS_INSTR );

  uint32_t te = millis(); // Stopwatch for measuring configuration time
 
  Serial.print(F("Bitstream size: "));
  Serial.print(num_read_total);
  Serial.println(F(" bytes"));
  Serial.print(F("Configuration done in "));
  Serial.print(te - ts);
  Serial.println(F(" msec"));
  printDivLine();
  fpgaConfigured = true;
}

#endif // JTAG_SEND_H
