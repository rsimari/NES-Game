/*

	Main driver program for NES game
	date: 2/22/18
	TODO: make UI, better sprite, 32x16 sprite
	      title screen
*/

#include <stdint.h>
#include <stddef.h>
#include "nes.h"
#include "reset.h"

// include level backgrounds
#include "Levels/levelT.h"
#include "Levels/level2.h"

const unsigned char * const LEVELS[] = {levelT, level2};

// include level collision maps
#include "Levels/levelT.csv"
#include "Levels/level2.csv"

const uint8_t PALETTE[] = {
	COLOR_WHITE, // background color first
	0x0c, 0x27, 0x37, // background palette 0
	0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0x1c, 0x0c, 0x27, 0x37, // sprite palette 0
	0,0,0,0,
	0,0,0,0,
	0,0,0,0
};

enum {Going_Up, Going_Down, Going_Left, Going_Right};
enum {Title_Mode, Play_Mode, End_Mode};

uint8_t time_min = 0;
uint8_t time_sec_low = 0;
uint8_t time_sec_high = 0;

uint8_t X = 80;
uint8_t Y = 80;

uint8_t player_state;
uint8_t level_status = 1;

uint8_t player_left_side;
uint8_t player_right_side;
uint8_t player_bottom;
uint8_t player_top;
uint8_t player_center_x;
uint8_t player_center_y;
uint8_t collision_row;
uint8_t collision_col;
uint8_t blocked;
uint8_t blocked_top;
uint8_t blocked_bot;

uint8_t start_x;    // coordinates of where to start on level
uint8_t start_y;
uint8_t end_x_min;  // corners of ending square the player will end on and go to next level
uint8_t end_x_max;
uint8_t end_y_min;
uint8_t end_y_max;

void reset_scroll(void);
void set_palette(void);
void init_player(void);
void screen_on(void);
void screen_off(void);
void update_time(void);
void input_handler(void);
void update_sprite(void);
void add_second(void);
void draw_background(void);
void get_player_border(void);
void collision_check_vert(void);
void collision_check_horiz(void);
void passed_level(void);

// main is called from reset.s
int main(void) {
	level_status = 0;
	start_y = 50;
	start_x = 50;
	end_x_min = 45;
	end_x_max = 55;
	end_y_min = 80;
	end_y_max = 90;
	screen_off();
	draw_background();
	set_palette();
	update_time();
	init_player();
	screen_on();

	// main game loop
	while(1) {
		WaitFrame(); // wait for vblank/nmi handler in reset.s to trigger
		if (player_tl.x <= end_x_max && player_tl.x >= end_x_min && player_tl.y <= end_y_max && player_tl.y >= end_y_min) {
			passed_level();
			screen_off();
			draw_background();
			Wait_Vblank();
			screen_on();
		}
		if (Frame_Number == 60) { // this runs once every second
			add_second();
			update_time();
			Frame_Number = 0;
		}
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

void set_palette(void) {
	// write palette data to BG_PALETTE
	PPU_ADDRESS = BG_PALETTE_HIGH;
	PPU_ADDRESS = BG_PALETTE_LOW;
	for (i = 0; i < sizeof(PALETTE); ++i) // always use ++i, instead of i++
		PPU_DATA = PALETTE[i];
	reset_scroll();
}

void init_player(void) {
	// set initial location for 2x2 player sprite
	X = start_x;
	Y = start_y;
	player_tl.x = MIN_X + X;
	player_tl.y = MIN_Y + Y;
	player_tl.attributes = 0x00;
	player_tl.tile_index = 0x80;

	player_tr.x = MIN_X + X + 8;
	player_tr.y = MIN_Y + Y;
	player_tr.attributes = 0x00;
	player_tr.tile_index = 0x81;

	player_bl.x = MIN_X + X;
	player_bl.y = MIN_Y + Y + 8;
	player_bl.attributes = 0x00;
	player_bl.tile_index = 0x90;

	player_br.x = MIN_X + X + 8;
	player_br.y = MIN_Y + Y + 8;
	player_br.attributes = 0x00;
	player_br.tile_index = 0x91;
	reset_scroll();
}

void screen_on(void) {
	// enable NMI and rendering (see https://wiki.nesdev.com/w/index.php/PPU_registers#PPUCTRL)
	PPU_CTRL = 0x80; // 1000 0000, turns NMI on
	PPU_MASK = 0x1e; // 0001 1110, show sprites and background
}

void screen_off(void) {
	PPU_CTRL = 0x00;
	PPU_MASK = 0x00;
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
		--player_tr.y;
		--player_tl.y;
		--player_bl.y;
		--player_br.y;
		player_state = Going_Up;
	}
	if (InputPort1 & BUTTON_DOWN) {
		++player_tr.y;
		++player_tl.y;
		++player_bl.y;
		++player_br.y;
		player_state = Going_Down;
	}

	collision_check_vert();

	if (InputPort1 & BUTTON_LEFT) {
		--player_tl.x;
		--player_tr.x;
		--player_bl.x;
		--player_br.x;
		player_state = Going_Left;
	}
	if (InputPort1 & BUTTON_RIGHT) {
		++player_tl.x;
		++player_tr.x;
		++player_bl.x;
		++player_br.x;
		player_state = Going_Right;
	}

	collision_check_horiz();
}

void update_sprite(void) {
	if (player_state == Going_Up) {
		// change sprite tile_index
		player_tl.tile_index = 0x86;
		player_tr.tile_index = 0x87;
	} else if (player_state == Going_Down) {
		// change sprite tile_index
		player_tl.tile_index = 0x84;
		player_tr.tile_index = 0x85;
	} else if (player_state == Going_Left) {
		// change sprite tile_index
		player_tl.tile_index = 0x80;
		player_tr.tile_index = 0x81;
	} else if (player_state == Going_Right) {
		// change sprite tile_index
		player_tl.tile_index = 0x82;
		player_tr.tile_index = 0x83;
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
	PPU_ADDRESS = NAMETABLE0_HIGH;
	PPU_ADDRESS = NAMETABLE0_LOW;

	if (level_status == 0) {
		UnRLE(levelT);
	} else if (level_status == 2) {
		UnRLE(level2);
	}
	reset_scroll();
}

void get_player_border(void) {
	// need to check for overflow
	player_left_side  = player_tl.x + PLAYER_LEFT_GAP;
	player_right_side = player_tr.x + PLAYER_WIDTH - PLAYER_RIGHT_GAP;
	player_top        = player_tl.y + PLAYER_TOP_GAP;
	player_bottom     = player_br.y + PLAYER_HEIGHT - PLAYER_BOT_GAP;
	player_center_x   = (player_left_side + player_right_side) >> 1;
	player_center_y   = (player_top + player_bottom) >> 1;
}

void collision_check_horiz(void) {
	get_player_border();
	blocked = 0;
	blocked_top = 0;
	blocked_bot = 0;
	if (InputPort1 & BUTTON_RIGHT) {
		// check right side
		collision_row = player_center_y >> 3;
		collision_col = player_right_side >> 3;
		if (level_status == 0) {
			blocked = c_map1[collision_row][collision_col];// | c_map1[collision_row-1][collision_col];
			blocked_top = c_map1[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map1[++collision_row][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];// | c_map2[collision_row-1][collision_col];
			blocked_top = c_map2[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map2[++collision_row][collision_col];
		}
		if (blocked != 0 || blocked_top != 0 || blocked_bot != 0) {
				--player_tl.x;
				--player_bl.x;
				--player_tr.x;
				--player_br.x;
		}
	}
	blocked = 0;
	blocked_top = 0;
	blocked_bot = 0;
	if (InputPort1 & BUTTON_LEFT) {
		// check left side
		collision_row = player_center_y >> 3;
		collision_col = player_left_side >> 3;
		if (level_status == 0) {
			blocked = c_map1[collision_row][collision_col];// | c_map1[collision_row-1][collision_col];
			blocked_top = c_map1[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map1[++collision_row][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];// | c_map2[collision_row-1][collision_col];
			blocked_top = c_map2[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map2[++collision_row][collision_col];
		}
		if (blocked != 0 || blocked_top != 0 || blocked_bot != 0) {
			++player_tl.x;
			++player_bl.x;
			++player_tr.x;
			++player_br.x;
		}
	}
}

void collision_check_vert(void) {
	get_player_border();
	if (InputPort1 & BUTTON_UP) {
		// check right side
		collision_row = player_top >> 3;
		collision_col = player_center_x >> 3;
		if (level_status == 0) {
			blocked = c_map1[collision_row][collision_col];// | c_map1[collision_row-1][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];// | c_map2[collision_row-1][collision_col];
		}
		if (blocked != 0) {
			++player_tl.y;
			++player_bl.y;
			++player_tr.y;
			++player_br.y;
		}
	}
	if (InputPort1 & BUTTON_DOWN) {
		// check left side
		collision_row = player_bottom >> 3;
		collision_col = player_center_x >> 3;
		if (level_status == 0) {
			blocked = c_map1[collision_row][collision_col];// | c_map1[collision_row-1][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];// | c_map2[collision_row-1][collision_col];
		}
		if (blocked != 0) {
			--player_tl.y;
			--player_bl.y;
			--player_tr.y;
			--player_br.y;
		}
	}
}

void passed_level(void) {
	++level_status;
}
