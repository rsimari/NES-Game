/*

	Main driver program for NES game
	date: 2/22/18
	TODO: collisions, draw background, make UI, better sprite, 32x16 sprite
	      title screen
*/

#include <stdint.h>
#include <stddef.h>
#include "nes.h"
#include "reset.h"

const uint8_t PALETTE[] = {
	COLOR_WHITE, // background color first
	COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, // background palette 0
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0x21, 0x0c, 0x27, 0x37, // sprite palette 0
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
};

const uint8_t SP_PALETTE[] = {
	0x21,
	0x0c, 0x27, 0x37
};

const uint8_t ATTRIBUTES[] = {
	0x44, // 0100 0100
	0xbb, // 1011 1011
	0x44, // 0100 0100
	0xbb  // 1011 1011
};

uint8_t time_min = 0;
uint8_t time_sec_low = 0;
uint8_t time_sec_high = 0;

void reset_scroll(void);
void screen_on(void);
void update_time(void);
void input_handler(void);
void add_second(void);
void draw_background(void);

// main is called from reset.s 
int main(void) {
	// write palette data to BG_PALETTE
	PPU_ADDRESS = BG_PALETTE_HIGH;
	PPU_ADDRESS = BG_PALETTE_LOW;
	for (i = 0; i < sizeof(PALETTE); ++i) // always use ++i, instead of i++
		PPU_DATA = PALETTE[i];
	reset_scroll();

	// PPU_ADDRESS = SP_PALETTE0_HIGH;
	// PPU_ADDRESS = SP_PALETTE0_LOW;
	// for (i = 0; i < sizeof(SP_PALETTE); ++i)
	// 	PPU_DATA = SP_PALETTE[i];
	// reset_scroll();

	update_time();

	// player.x = MIN_X + 20;
	// player.y = MIN_Y + 20;
	// player.tile_index = PLAYER_TILE;
	// sprite_t tl;
	tl.x = MIN_X + 20;
	tl.y = MIN_Y + 20;
	tl.attributes = 0x00;
	tl.tile_index = 0x80;

	tr.x = MIN_X + 28;
	tr.y = MIN_Y + 20;
	tr.attributes = 0x00;
	tr.tile_index = 0x81;

	bl.x = MIN_X + 20;
	bl.y = MIN_Y + 28;
	bl.attributes = 0x00;
	bl.tile_index = 0x90;

	br.x = MIN_X + 28;
	br.y = MIN_Y + 28;
	br.attributes = 0x00;
	br.tile_index = 0x91;

	// meta_sprite_t player;

	screen_on();

	// main game loop
	while(1) {
		WaitFrame(); // wait for vblank/nmi handler in reset.s to trigger
		if (Frame_Number == 60) { // this runs once every second
			add_second();
			Frame_Number = 0;
		}
		reset_scroll();
		update_time();
		// input_handler();
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
	/*
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
		if (player.x > MIN_X + SPRITE_WIDTH) {
			--player.x;
		}
	}
	if (InputPort1 & BUTTON_RIGHT) {
		if (player.x < MAX_X + SPRITE_WIDTH) {
			++player.x;
		}
	}
	*/
}

void add_second(void) {
	if (time_sec_low < 9)
		++time_sec_low;
	else if (time_sec_low == 9) {
		time_sec_low = 0;
		if (time_sec_high < 5)
			++time_sec_high;
		else if (time_sec_high == 5) {
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

void draw_background(void) {

	reset_scroll();
}

