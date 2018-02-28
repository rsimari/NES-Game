/*
	defines some useful NES constants
*/

#ifndef NES_H_
#define NES_H_

#include <stdint.h>

// helpful definitions for PPU addresses
#define PPU_CTRL    *((uint8_t*)0x2000)
#define PPU_MASK    *((uint8_t*)0x2001)
#define PPU_STATUS  *((uint8_t*)0x2002)
#define PPU_SCROLL  *((uint8_t*)0x2005)
#define PPU_ADDRESS *((uint8_t*)0x2006)
#define PPU_DATA    *((uint8_t*)0x2007)

// addresses of background palettes
#define BG_PALETTE_HIGH  0x3f // universal background color
#define BG_PALETTE_LOW   0x00

// sprite palette
#define SP_PALETTE_HIGH 0x3f
#define SP_PALETTE_LOW  0x10

// address of start of nametable 0-3, $2000-$2c00
#define NAMETABLE0_HIGH 0x20
#define NAMETABLE0_LOW  0x00
#define NAMETABLE1_HIGH 0x24
#define NAMETABLE1_LOW  0x00
#define NAMETABLE2_HIGH 0x28
#define NAMETABLE2_LOW  0x00
#define NAMETABLE3_HIGH 0x2c
#define NAMETABLE3_LOW  0x00

// addresses of attribute tables, 
#define ATTRTABLE0_HIGH 0x23 // each byte represents a 32x32
#define ATTRTABLE0_LOW  0xc0 // pixel part of screen from $23c0 - $23ff
#define ATTRTABLE1_HIGH 0x27
#define ATTRTABLE1_LOW  0xc0
#define ATTRTABLE2_HIGH 0x2b
#define ATTRTABLE2_LOW  0xc0
#define ATTRTABLE3_HIGH 0x2f
#define ATTRTABLE3_LOW  0xc0

// helpful color definitions
#define COLOR_BLACK 0x0f
#define COLOR_WHITE 0x20
#define COLOR_BLUE  0x12
#define COLOR_GREEN 0x1a
#define COLOR_RED   0x17

// helpful screen dimension definitions
#define MIN_X 0
#define MAX_X 256
#define MIN_Y 0
#define MAX_Y 239

// .chr mappings
#define PLAYER_HEIGHT    0x10
#define PLAYER_WIDTH     0x08
#define NUMBER_0         0x30
#define PLAYER_LEFT_GAP  0x03
#define PLAYER_RIGHT_GAP 0x03
#define PLAYER_TOP_GAP   0x00
#define PLAYER_BOT_GAP   0x00

// button addres mappings
#define BUTTON_RIGHT  0x01
#define BUTTON_LEFT   0x02
#define BUTTON_DOWN   0x04
#define BUTTON_UP     0x08
#define BUTTON_START  0x10
#define BUTTON_SELECT 0x20
#define BUTTON_B      0x40
#define BUTTON_A      0x80

#endif