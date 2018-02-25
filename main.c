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

enum {Going_Up, Going_Down, Going_Left, Going_Right};

uint8_t time_min = 0;
uint8_t time_sec_low = 0;
uint8_t time_sec_high = 0;

uint8_t state;

void reset_scroll(void);
void screen_on(void);
void update_time(void);
void input_handler(void);
void update_sprite(void);
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
	update_time();

	// set initial location for 2x2 player sprite
	player_tl.x = MIN_X + 20;
	player_tl.y = MIN_Y + 20;
	player_tl.attributes = 0x00;
	player_tl.tile_index = 0x80;

	player_tr.x = MIN_X + 28;
	player_tr.y = MIN_Y + 20;
	player_tr.attributes = 0x00;
	player_tr.tile_index = 0x81;

	player_bl.x = MIN_X + 20;
	player_bl.y = MIN_Y + 28;
	player_bl.attributes = 0x00;
	player_bl.tile_index = 0x90;

	player_br.x = MIN_X + 28;
	player_br.y = MIN_Y + 28;
	player_br.attributes = 0x00;
	player_br.tile_index = 0x91;

	screen_on();

	// main game loop
	while(1) {
		WaitFrame(); // wait for vblank/nmi handler in reset.s to trigger
		if (Frame_Number == 60) { // this runs once every second
			add_second();
			Frame_Number = 0;
		}
		update_time();
		input_handler();
		update_sprite();
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
		// check top 2 sprites
		if (player_tl.y > MIN_Y + SPRITE_HEIGHT && player_tr.y > MIN_Y + SPRITE_HEIGHT) {
			--player_tr.y;
			--player_tl.y;
			--player_bl.y;
			--player_br.y;
			state = Going_Up;
		}
	}
	if (InputPort1 & BUTTON_DOWN) {
		// check bottom 2 sprites
		if (player_bl.y < MAX_Y - SPRITE_HEIGHT && player_br.y < MAX_Y - SPRITE_HEIGHT) {
			++player_tr.y;
			++player_tl.y;
			++player_bl.y;
			++player_br.y;
			state = Going_Down;
		}
	}
	if (InputPort1 & BUTTON_LEFT) {
		// check left 2 sprites
		if (player_tl.x > MIN_X + SPRITE_WIDTH && player_bl.x > MIN_X + SPRITE_WIDTH) {
			--player_tl.x;
			--player_tr.x;
			--player_bl.x;
			--player_br.x;
			state = Going_Left;
		}
	}
	if (InputPort1 & BUTTON_RIGHT) {
		// check right 2 sprites
		if (player_tr.x < MAX_X + SPRITE_WIDTH && player_br.x < MAX_X + SPRITE_WIDTH) {
			++player_tl.x;
			++player_tr.x;
			++player_bl.x;
			++player_br.x;
			state = Going_Right;
		}
	}
}

void update_sprite(void) {
	if (state == Going_Up) {
		// change sprite tile_index
	} else if (state == Going_Down) {
		// change sprite tile_index
	} else if (state == Going_Left) {
		// change sprite tile_index
		player_tr.tile_index = 0x81;
		player_tl.tile_index = 0x80;
	} else if (state == Going_Right) {
		// change sprite tile_index
		player_tr.tile_index = 0x83;
		player_tl.tile_index = 0x82;
	}
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

