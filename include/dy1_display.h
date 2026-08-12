#pragma once

#include <Arduino.h>

// ################################################################################
//
//     ######  ###  #####  ######  #          #    #     # 
//     #     #  #  #     # #     # #         # #    #   #  
//     #     #  #  #       #     # #        #   #    # #   
//     #     #  #   #####  ######  #       #     #    #    
//     #     #  #        # #       #       #######    #    
//     #     #  #  #     # #       #       #     #    #    
//     ######  ###  #####  #       ####### #     #    #    
//                                                         
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
#ifdef USE_DY1_DISPLAY
  #define DY1_LATCH_PIN 16 // Register-Strobe für 74HC595
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
  uint8_t displ_arr[8]; // ausreichend für 8 Digits
#endif

void spi_send_displ_arr() {
  #ifdef USE_DY1_DISPLAY
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
  #endif
}


void clear_disp(uint8_t start_pos) {
  #ifdef USE_DY1_DISPLAY
    for (uint8_t i = start_pos; i <= DIGIT_MAX; i++) {
      displ_arr[i] = 0x00;
    }
    spi_send_displ_arr();
  #endif
}


void set_letter(uint8_t pos, unsigned char letter) {
// normale Anzeige, alle VFDs, <letter> als ASCII
  #ifdef USE_DY1_DISPLAY
    if (letter >= 96) letter = letter - 32; // keine Kleinbuchstaben
    uint8_t segments = letter2segm[letter - 32];
    displ_arr[pos] = segments;
  #endif
}

void set_dp(uint8_t pos) {
  #ifdef USE_DY1_DISPLAY
    // Dezimalpunkt an dieser Position einschalten
    displ_arr[pos] = displ_arr[pos] | seg_dp;
  #endif
}


void set_number(int32_t number, int dp_pos, bool ripple_blank = true) {
  // dp_pos = -1: keine Dezimalstelle, 0..DIGIT_MAX: Dezimalpunkt an dieser Position
  // ripple_blank = true: führende Nullen werden als Leerzeichen angezeigt
  // ripple_blank = false: führende Nullen werden als "0" angezeigt
  // Ein- oder mehrstellige Zahl <number> anzeigen
  #ifdef USE_DY1_DISPLAY
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
  #endif
}


void set_static_message(String msg) {
  #ifdef USE_DY1_DISPLAY
    for (uint8_t i = 0; (i < msg.length()) && (i <= DIGIT_MAX); i++) {
      set_letter(i, msg.charAt(i));
    }
    spi_send_displ_arr();
  #endif
}

void set_rdy_message() {
  #ifdef USE_DY1_DISPLAY
    // Anzeige "rdy" auf OHO-Display
    set_letter(0, 'r');
    set_letter(1, 'd');
    set_letter(2, 'y');
    spi_send_displ_arr();
  #endif
}

void test_display() {
  #ifdef USE_DY1_DISPLAY
    Serial.println(F("Testing DY1 display..."));
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
    delay(500);
  #endif
}

