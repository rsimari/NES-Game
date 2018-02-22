/*

	Main driver program for NES game
	date: 2/22/18

*/

// helpful definitions for PPU addresses
#define PPU_CTRL    *((uint8_t*)0x2000)
#define PPU_MASK    *((uint8_t*)0x2001)
#define PPU_STATUS  *((uint8_t*)0x2002)
#define PPU_SCROLL  *((uint8_t*)0x2005)
#define PPU_ADDRESS *((uint8_t*)0x2006)
#define PPU_DATA    *((uint8_t*)0x2007)

// address of background palette
#define BG_PALETTE_HIGH 0x3f
#define BG_PALETTE_LOW  0x00

// address of start of nametable 0, $2000
#define NM0_START_HIGH 0x20
#define NM0_START_LOW  0x00

// helpful color definitions
#define COLOR_BLACK 0x0f
#define COLOR_WHITE 0x20
#define COLOR_BLUE  0x12
#define COLOR_GREEN 0x1a
#define COLOR_RED   0x17

// helpful C constants and variable types
#include <stddef.h>
#include <stdint.h>

// puts an iterator variable in the zeropage for fast access
#pragma bss-name(push, "ZEROPAGE")
size_t i;
#pragma bss-name(pop)

const uint8_t PALETTE[] = {
	COLOR_BLACK, // background color first
	COLOR_WHITE, COLOR_WHITE, COLOR_WHITE // background palette 0
};

const char TEXT [] = "Hello, World!";

void reset_scroll(void);

// main is called from reset.s 
int main(void) {
	// write palette data to BG_PALETTE
	PPU_ADDRESS = BG_PALETTE_HIGH;
	PPU_ADDRESS = BG_PALETTE_LOW;
	for (i = 0; i < sizeof(PALETTE); ++i) // always use ++i, instead of i++
		PPU_DATA = PALETTE[i];

	// write text to nametable 0
	PPU_ADDRESS = NM0_START_HIGH + 0x01; // offset to put in middle of screen
	PPU_ADDRESS = NM0_START_LOW  + 0xca;
	for (i = 0; i < sizeof(TEXT); ++i)
		PPU_DATA = TEXT[i];

	reset_scroll();

	// enable NMI and rendering (see https://wiki.nesdev.com/w/index.php/PPU_registers#PPUCTRL)
	PPU_CTRL = 0x80; // 1000 0000, turns NMI on
	PPU_MASK = 0x1e; // 0001 1110, show sprites and background

	// main game loop
	while(1) {}

	return 0;
}


void reset_scroll(void) {
	// sets scroll to 0,0 position, just like PPU_ADDRESS it increments after a write
	PPU_SCROLL = 0x00;
	PPU_SCROLL = 0x00;
}

