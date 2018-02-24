/*

	Main driver program for NES game
	date: 2/22/18

*/

#include <stdint.h>
#include <stddef.h>
#include "nes.h"
#include "reset.h"

const uint8_t PALETTE[] = {
	COLOR_BLACK, // background color first
	COLOR_WHITE, COLOR_WHITE, COLOR_WHITE // background palette 0
};

// const char TEXT [] = "Hello, World!";
const char TEXT [] = {
	72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100 // Hello World
};

const char TIME [] = { 0x30, 0x3a, 0x30, 0x30 }; // 0:00

void reset_scroll(void);
void screen_on(void);
void update_time(void);
void input_handler(void);

// main is called from reset.s 
int main(void) {
	// write palette data to BG_PALETTE
	PPU_ADDRESS = BG_PALETTE_HIGH;
	PPU_ADDRESS = BG_PALETTE_LOW;
	for (i = 0; i < sizeof(PALETTE); ++i) // always use ++i, instead of i++
		PPU_DATA = PALETTE[i];

	// write text to nametable 0
	// PPU_ADDRESS = NAMETABLE0_HIGH + 0x01; // offset to put in middle of screen
	// PPU_ADDRESS = NAMETABLE0_LOW  + 0xca;
	// for (i = 0; i < sizeof(TEXT); ++i)
	// 	PPU_DATA = TEXT[i];
	update_time();
	reset_scroll();

	player.x = MIN_X + 20;
	player.y = MIN_Y + 20;
	player.tile_index = PLAYER_TILE;

	screen_on();

	// main game loop
	while(1) {
		WaitFrame(); // wait for vblank/nmi handler in reset.s to trigger
		reset_scroll();

		input_handler();
	}

	return 0;
}


void reset_scroll(void) {
	// sets scroll to 0,0 position, just like PPU_ADDRESS it increments after a write
	PPU_SCROLL = 0x00;
	PPU_SCROLL = 0x00;
}

void screen_on(void) {
	// enable NMI and rendering (see https://wiki.nesdev.com/w/index.php/PPU_registers#PPUCTRL)
	PPU_CTRL = 0x80; // 1000 0000, turns NMI on
	PPU_MASK = 0x1e; // 0001 1110, show sprites and background
}

void update_time(void) {
	// updates time on screen
	PPU_ADDRESS = NAMETABLE0_HIGH + 0x00; // top-right of screen
	PPU_ADDRESS = NAMETABLE0_LOW  + 0x59;
	for (i = 0; i < sizeof(TIME); ++i)
		PPU_DATA = TIME[i];
}

void input_handler(void) {
	if (InputPort1 & BUTTON_UP) {
		if (player.y > MIN_Y + SPRITE_HEIGHT) {
			--player.y;
		}
	}
	if (InputPort1 & BUTTON_DOWN) {
		if (player.y < MAX_Y - SPRITE_HEIGHT) {
			++player.y;
		}
	}
	if (InputPort1 & BUTTON_LEFT) {
		if (player.x > MIN_X + SPRITE_HEIGHT) {
			--player.x;
		}
	}
	if (InputPort1 & BUTTON_RIGHT) {
		if (player.x < MAX_Y + SPRITE_HEIGHT) {
			++player.x;
		}
	}
}

