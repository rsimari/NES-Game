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

uint8_t time_min = 0;     // 0 in the .chr file
uint8_t time_sec_low = 0;
uint8_t time_sec_high = 0;

void reset_scroll(void);
void screen_on(void);
void update_time(void);
void input_handler(void);
void add_second(void);

// main is called from reset.s 
int main(void) {
	// write palette data to BG_PALETTE
	PPU_ADDRESS = BG_PALETTE_HIGH;
	PPU_ADDRESS = BG_PALETTE_LOW;
	for (i = 0; i < sizeof(PALETTE); ++i) // always use ++i, instead of i++
		PPU_DATA = PALETTE[i];
	reset_scroll();
	update_time();

	player.x = MIN_X + 20;
	player.y = MIN_Y + 20;
	player.tile_index = PLAYER_TILE;

	screen_on();

	// main game loop
	while(1) {
		WaitFrame(); // wait for vblank/nmi handler in reset.s to trigger
		if (Frame_Number == 60) {
			add_second();
			Frame_Number = 0;
		}
		reset_scroll();
		update_time();
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
	PPU_DATA = NUMBER_0 + time_min;
	PPU_DATA = 0x3a; // ':'
	PPU_DATA = NUMBER_0 + time_sec_high;
	PPU_DATA = NUMBER_0 + time_sec_low;
	reset_scroll();
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

void add_second(void) {
	if (time_sec_low < 9)
		++time_sec_low;
	else if (time_sec_low == 9) {
		time_sec_low = 0;
		if (time_sec_high < 9)
			++time_sec_high;
		else if (time_sec_high == 9) {
			time_sec_high = 0;
			if (time_min < 9) 
				++time_min;
			else {
				time_min = 0;
				time_sec_high = 0;
				time_sec_low = 0;
			}
		}
	}
}

