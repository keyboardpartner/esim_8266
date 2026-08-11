#ifndef JTAG_SEND_H
#define JTAG_SEND_H
//////////////////////////////////////////////////////////////////////////
// Author: RSP @ Embedded Systems Lab (ESL), KMUTNB, Bangkok / Thailand
// Date: 2017-07-06
// Arduino IDE: v1.8.2 + esp8266 v2.3.0
// MCU Boards with ESP-12E
// Objective: This sketch shows how to use an ESP8266 module to
//   configure the Xilinx Spartan-6 FPGA device using the JTAG port.
//   The bitstream file ("TOP.BIN") and its associated MD5 checksum file ("TOP.MD5")
//   are stored in a microSD attached to the ESP8266 via the SPI bus.
//   The MD5 checksum calculation is performed first before loading the bitstream.
//   This sketch can successfully load the bitstream into the Xilinx Spartan 6SLX9
//   FPGA device on the Mojo v3 board.
//////////////////////////////////////////////////////////////////////////


#include <Arduino.h>
#include <LittleFS.h>
#include <cstdlib>
#include <cstdio>
#include "global_vars.h" // for #defines

// ESP8266 Pins for JTAG: 
const int TCK_PIN = 5; // D1 / GPIO-5 (output)
const int TDO_PIN = 4; // D2 / GPIO-4 (input)
const int TDI_PIN = 0; // D3 / GPIO-0 (output)
const int TMS_PIN = 2; // D4 / GPIO-2 (output)

#define XILINX_SPARTAN6
#define USE_FAST_IO
#define MAX_BUF_SIZE   (2048)

// global variables
char sbuf[64];
boolean config_fpga = false;
char buf[MAX_BUF_SIZE];

#ifdef XILINX_SPARTAN6
 // see: Spartan-6 FPGA Configuration User Guide UG380 (v2.10) March 31, 2017
 #define XILINX_IR_LEN             (6)
 #define XILINX_USER1_INSTR        (0x02)  // 000010
 #define XILINX_USER2_INSTR        (0x03)  // 000011
 #define XILINX_USER3_INSTR        (0x1A)  // 011010
 #define XILINX_USER4_INSTR        (0x1B)  // 011011
 #define XILINX_CFG_OUT_INSTR      (0x04)  // 000100
 #define XILINX_CFG_IN_INSTR       (0x05)  // 000101
 #define XILINX_BYPASS_INSTR       (0x1F)  // 111111
 #define XILINX_IDCODE_INSTR       (0x09)  // 001001
 #define XILINX_USERCODE_INSTR     (0x08)  // 001000
 #define XILINX_JPROGRAM_INSTR     (0x0B)  // 001011
 #define XILINX_JSTART_INSTR       (0x0C)  // 001100
 #define XILINX_JSHUTDOWN_INSTR    (0x0D)  // 001101 
 #define XILINX_S6LX9_IDCODE       (0x04001093)  // Spartan-6 LX9 FPGA device ID code
#endif

///////////////////////////////////////////////////////////////////////

#ifdef USE_FAST_IO

inline void jtag_clk( int tms ) {
  GPOC = (1<<TCK_PIN);
  if ( tms ) 
    GPOS = (1<<TMS_PIN);
  else
    GPOC = (1<<TMS_PIN);
  GPOS = (1<<TCK_PIN);
}

inline int jtag_clk_data( int tms, int tdi ) {
  GPOC = (1<<TCK_PIN);  
  if ( tdi ) 
    GPOS = (1<<TDI_PIN);
  else
    GPOC = (1<<TDI_PIN);
  if ( tms ) 
    GPOS = (1<<TMS_PIN);
  else
    GPOC = (1<<TMS_PIN);
  GPOS = (1<<TCK_PIN);  
  return (GPI >> TDO_PIN) & 1;
}

void jtag_goto_runtest_idle() {
  GPOC = (1<<TCK_PIN);  
  for ( int i=0; i < 8; i++ ) {
     jtag_clk(1);
  }
  jtag_clk(0);  // goto Run-Test/Idle
}

#else

inline void jtag_clk( int tms ) {
  digitalWrite( TCK_PIN, 0 );
  digitalWrite( TMS_PIN, tms );
  digitalWrite( TCK_PIN, 1 );
}

inline int jtag_clk_data( int tms, int tdi ) {
  int tdo;
  digitalWrite( TCK_PIN, 0 );
  digitalWrite( TDI_PIN, tdi );
  digitalWrite( TMS_PIN, tms ); // TMS must be stable before the rising edge of TCK
  digitalWrite( TCK_PIN, 1 );
  tdo = digitalRead( TDO_PIN );
  return tdo;
}

void jtag_goto_runtest_idle() {
  digitalWrite( TCK_PIN, 0 );
  for ( int i=0; i < 10; i++ ) {
     jtag_clk(1);
  }
  jtag_clk(0);  // goto Run-Test/Idle
}
#endif
///////////////////////////////////////////////////////////////////////

void jtag_load_ir( uint32_t instr, int ir_len ) {
  int tdi, tms;

  // start from Run-Test/Idle state
  jtag_clk(1);  // goto Select-DR-Scan
  jtag_clk(1);  // goto Select-IR-Scan
  jtag_clk(0);  // goto Capture-IR
  jtag_clk(0);  // goto shift-IR

  for ( int i=0; i < ir_len; i++ ) {
     tdi = (instr & 1);
     tms = (i==(ir_len-1)) ? 1 : 0;
     jtag_clk_data( tms, tdi ); // goto Exit1-IR or shift-DR
     instr = (instr >> 1);
  }
  jtag_clk(1);  // goto Update-IR
  jtag_clk(0);  // goto Run-Test/Idle
}

uint32_t jtagReadIDcode( ) {
  int tdi, tms, tdo;
  uint32_t idcode = 0;

  jtag_goto_runtest_idle();
  jtag_load_ir( XILINX_IDCODE_INSTR, XILINX_IR_LEN );

  // start from Run-Test/Idle state
  jtag_clk(1);  // goto Select-DR-Scan
  jtag_clk(0);  // goto Capture-DR
  jtag_clk(0);  // goto Shift-DR

  // now in Shift-DR state
  tdi = 0;
  for ( int i=0; i < 32; i++ ) {
     tms = (i==31) ? 1 : 0;
     tdo = jtag_clk_data( tms, tdi ); // goto Exit1-DR or Shift-DR
     idcode = (tdo << 31) | (idcode >> 1);
  }
  // now in Exit1-DR state
  jtag_clk(1);  // goto Update-DR
  jtag_clk(0);  // goto Run-Test/Idle

  return idcode;
}

boolean jtagCheckBitstreamFile(const String &config_file) {
  String str;
  boolean file_valid = false;
  if ( LittleFS.exists(config_file.c_str()) ) {
     File f = LittleFS.open( config_file.c_str(), "r" );
     if ( f ) {
       int bytes_read_total = 0;
       str = "File size: ";
       str += f.size();
       Serial.println( str );
       f.close();

       str = "Number of bytes read: ";
       str += bytes_read_total;
       Serial.println( str ); 
       file_valid = true;     
     }
  } 
  else {
    str = "Cannot open file (no existing): ";
    str += config_file;
    Serial.println( str );
  }
  return file_valid;
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


void jtagConfigure(const String &config_file) {
  Serial.println();
  printCenteredSerial(F("FPGA Config"));
  int num_read_total = 0;
  String str;
  uint32_t ts = millis();
  
  File f = LittleFS.open(config_file.c_str(), "r");
  if (!f) {
    str = "Cannot open bitstream file: ";
    str += config_file;
    Serial.println( str );
    return;
  }
  int file_size = f.size();  // get file size (in bytes)

  // goto RunTest/Idle
  jtag_goto_runtest_idle();
  Serial.print( "Sending File: " );
  Serial.println( config_file );

  DPRINTLNF( "Send JSHUTDOWN instruction" );
  jtag_load_ir( XILINX_JSHUTDOWN_INSTR, XILINX_IR_LEN );
  for ( int i=0; i < 32; i++ ) {
     jtag_clk(0);  // stay at Run-Test/Idle state
  }

  DPRINTLNF( "Send CFG_IN instruction" );
  jtag_load_ir( XILINX_CFG_IN_INSTR, XILINX_IR_LEN );

  // start from Run-Test/Idle state
  jtag_clk(1);  // goto Select-DR-Scan
  jtag_clk(0);  // goto Capture-DR
  jtag_clk(0);  // goto Shift-DR

  // now in Shift-DR state
  int tdi, tms, chunk_count = 0;
  Serial.print( "Progress: " );
  while ( f.available() ) {
    int chunk_size = f.read(reinterpret_cast<uint8_t *>(buf), MAX_BUF_SIZE-1);
    num_read_total += chunk_size;
    boolean last_bit;
    boolean last_chunk = (num_read_total==file_size);
    for ( int j=0; j < chunk_size; j++ ) {    // for each byte of the chunk data
      boolean last_byte = last_chunk && (j==chunk_size-1);
      uint8_t data = buf[j];                  // get the next byte
      for ( int i=0; i < 8; i++ ) {           // for each bit of the byte data
        tdi = (data & 0x80) ? 1 : 0;          // get the MSB bit
        data = data << 1;                     // shift-to-left
        last_bit = last_byte && (i==7);       // check for the last bit
        tms = ((i==7) && last_bit ) ? 1 : 0;  // goto Exit1-DR (1) or Shift-DR (0)
        jtag_clk_data( tms , tdi );
      }
    }
    delay(0); yield();
    if (chunk_count % 16 == 0) {
      Serial.print(".");
    }
    chunk_count++;
  }
  Serial.println(" Done.");
  // now in Exit1-DR state
  jtag_clk(1);  // goto Update-DR
  jtag_clk(0);  // goto Run-Test/Idle

  DPRINTLNF("send JSTART instruction" );
  jtag_load_ir( XILINX_JSTART_INSTR, XILINX_IR_LEN );

  // toggle TCK for startup sequence
  for ( int i=0; i < 32; i++ ) {
     jtag_clk(0);  // stay at Run-Test/Idle state
  }
 
  f.close();
  
  str = "Bitstream size: ";
  str += num_read_total;
  str += " bytes";
  Serial.println( str );
  str = "Configuration done in " ;
  str += millis() - ts;
  str += " msec";
  Serial.println( str );
  printDivLine();
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
// JTAG Port Setup
// ##############################################################################


void jtagSetup() {
  // configure GPIO pins for JTAG link 
  pinMode( TCK_PIN, OUTPUT );
  pinMode( TMS_PIN, OUTPUT );
  pinMode( TDI_PIN, OUTPUT );
  delay(10);
}

#endif // JTAG_SEND_H
