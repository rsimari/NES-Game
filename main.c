/*

	Main driver program for NES game
	date: 2/22/18
	// change palette for some items on screen to look better,
	// setup logic for tele's and internal switching
*/

#include <stdint.h>
#include <stddef.h>
#include "nes.h"
#include "reset.h"

// include level backgrounds
#include "Levels/empty.h"
#include "Levels/level1.h"
#include "Levels/levelT.h"
#include "Levels/level3.h"
#include "Levels/level4.h"
#include "Levels/level5.h"
#include "Levels/end.h"

// level1, intro
// level2, maze
// level3, two exits
// level4, two exits w/ twist
// level5, many exits
const unsigned char * const LEVELS[] = {level1, level2, level3, level4, level5};

// include level collision maps
#include "Levels/level1.csv"
#include "Levels/levelT.csv"
#include "Levels/level3.csv"
#include "Levels/level4.csv"
#include "Levels/level5.csv"


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

enum {Going_Up, Going_Down, Going_Left, Going_Right, Celebrate};
enum {Title, Level, Level_Passed, End, Level_Intro};

uint8_t time_min = 0;
uint8_t time_sec_low = 0;
uint8_t time_sec_high = 0;
uint8_t timer = 0;

uint8_t X = 80;
uint8_t Y = 80;

uint8_t player_state;
uint8_t level_status = 1;
uint8_t game_state;

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

uint8_t level_switch = 0;
uint8_t level_target = 0; 

uint8_t start_x;    // coordinates of where to start on level
uint8_t start_y;
uint8_t end_x_min;  // corners of ending square the player will end on and go to next level
uint8_t end_x_max;
uint8_t end_y_min;
uint8_t end_y_max;
uint8_t end_drawn = 0;

void reset_scroll(void);
void set_palette(void);
void init_player(void);
void set_player(void);
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
void level_intro(void);
void level3_tele_logic(void);
void level4_tele_logic(void);
void level5_tele_logic(void);
void draw_title(void);
void clear_nametable(void);
void draw_end(void);

// main is called from reset.s
int main(void) {
	level_status = 0;
	game_state = Title;
	start_x = 112;
	start_y = 104;
	end_x_min = 144;
	end_x_max = 160;
	end_y_min = 96;
	end_y_max = 120;
	screen_off();
	// draw_title();
	set_palette();
	draw_title();
	screen_on();

	// main game loop
	while(1) {
		WaitFrame(); // wait for vblank/nmi handler in reset.s to trigger

		if (game_state == Title) {
		    if (InputPort1 & BUTTON_START) {
		      timer = 0;
		      game_state = Level_Intro;
		      screen_off();
		      clear_nametable();
		      Wait_Vblank();
		      screen_on();
		      level_intro();
		    }
		    Frame_Number = 0;
		}		
		// title done... now start levels
		if (game_state == Level_Intro) {
			if (Frame_Number == 60) { // this runs once every second
				++timer;
				if (timer >= 3) {
					timer = 0;
					game_state = Level;
					screen_off();
					clear_nametable();
					Wait_Vblank();
					screen_on();

					init_player();
					// draw the next level background
					screen_off();
					draw_background();
					Wait_Vblank();
					screen_on();
				}
				update_time();
				Frame_Number = 0;
			}
		}		

		if (game_state == Level) {
			input_handler();
			update_sprite();
			reset_scroll();
			// for simple levels
			if ((player_tl.x <= end_x_max && player_tl.x + PLAYER_WIDTH >= end_x_min && 
				player_tl.y <= end_y_max && player_tl.y + PLAYER_HEIGHT >= end_y_min &&
				BUTTON_A & InputPort1)) {
				game_state = Level_Passed;
			// for levels with teleporters
			} else if (level_status == 2) {
				// check for teleports
				level3_tele_logic();
			} else if (level_status == 3) {
				level4_tele_logic();
			} else if (level_status == 4) {
				level5_tele_logic();
			}
			reset_scroll();
		}

		if (game_state == Level_Passed) {
			player_state = Celebrate;
			update_sprite();
			game_state = Level_Intro;
			passed_level();
			// set new end_x ... and player position for next level
			if (level_status == 1) {
				start_x = 48; // levelT
				start_y = 64;
				end_x_min = 136;
				end_x_max = 144;
				end_y_min = 176;
				end_y_max = 192;
				level_switch = 0;
			} else if (level_status == 2) {
				start_x = 48; // level 3
				start_y = 88;
				level_switch = 0;
			} else if (level_status == 3) {
				start_x = 48; // level 4
				start_y = 88;
				level_switch = 0;
			} else if (level_status == 4) {
				start_x = 24; // level 5
				start_y = 40;
				level_switch = 0;
			} else if (level_status == 5) {
				game_state = End;
			}
			if (game_state != End) {
				// draw the next level intro
				screen_off();
				clear_nametable();
				Wait_Vblank();
				screen_on();
				level_intro();
				set_player();
			}
		}

		if (game_state == End) {
			input_handler();
			update_sprite();
			if (end_drawn == 0) {
				draw_end();
				end_drawn = 1;
				start_x = 40;
				start_y = 40;
				set_player();
			}
			if (BUTTON_START & InputPort1) {
				game_state = Title;
				level_status = 0;
				start_x = 112;
				start_y = 104;
				end_x_min = 144;
				end_x_max = 160;
				end_y_min = 96;
				end_y_max = 120;
			}
		}

		if (game_state != End && Frame_Number == 60) {
			add_second();
			update_time();
			Frame_Number = 0;
		}
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

void set_player(void) {
	// set player location for 2x2 player sprite
	X = start_x;
	Y = start_y;
	player_tl.x = MIN_X + X;
	player_tl.y = MIN_Y + Y;

	player_tr.x = MIN_X + X + 8;
	player_tr.y = MIN_Y + Y;

	player_bl.x = MIN_X + X;
	player_bl.y = MIN_Y + Y + 8;

	player_br.x = MIN_X + X + 8;
	player_br.y = MIN_Y + Y + 8;
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
	if (game_state != End) {
		if (player_state == Going_Up) {
			// change sprite tile_index
			player_tl.tile_index = 0x86;
			player_tr.tile_index = 0x87;
			player_bl.tile_index = 0x90;
			player_br.tile_index = 0x91;
		} else if (player_state == Going_Down) {
			// change sprite tile_index
			player_tl.tile_index = 0x84;
			player_tr.tile_index = 0x85;
			player_bl.tile_index = 0x90;
			player_br.tile_index = 0x91;
		} else if (player_state == Going_Left) {
			// change sprite tile_index
			player_tl.tile_index = 0x80;
			player_tr.tile_index = 0x81;
			player_bl.tile_index = 0x90;
			player_br.tile_index = 0x91;
		} else if (player_state == Going_Right) {
			// change sprite tile_index
			player_tl.tile_index = 0x82;
			player_tr.tile_index = 0x83;
			player_bl.tile_index = 0x90;
			player_br.tile_index = 0x91;
		} else if (player_state == Celebrate) {
			player_tl.tile_index = 0x88;
			player_tr.tile_index = 0x89;
			player_bl.tile_index = 0x98;
			player_br.tile_index = 0x99;
		}
	} else {
		player_tl.tile_index = 0x88;
		player_tr.tile_index = 0x89;
		player_bl.tile_index = 0x98;
		player_br.tile_index = 0x99;		
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
		UnRLE(level1);
	} else if (level_status == 1) {
		UnRLE(level2);
	} else if (level_status == 2) {
		UnRLE(level3);
	} else if (level_status == 3) {
		UnRLE(level4);
	} else if (level_status == 4) {
		UnRLE(level5);
	}
	reset_scroll();
}

void get_player_border(void) {
	// need to check for overflow
	player_left_side  = player_tl.x + PLAYER_LEFT_GAP;
	player_right_side = player_tr.x + PLAYER_WIDTH - PLAYER_RIGHT_GAP;
	player_top        = player_tl.y + PLAYER_TOP_GAP - 8;
	player_bottom     = player_br.y - PLAYER_BOT_GAP;
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
			blocked = c_map1[collision_row][collision_col];
			blocked_top = c_map1[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map1[++collision_row][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];
			blocked_top = 0;//c_map2[--collision_row][collision_col];
			// ++collision_row;
			blocked_bot = 0;//c_map2[++collision_row][collision_col];
		} else if (level_status == 2) {
			blocked = c_map3[collision_row][collision_col];
			blocked_top = c_map3[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map3[++collision_row][collision_col];
		} else if (level_status == 3) {
			blocked = c_map4[collision_row][collision_col];
			blocked_top = c_map4[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map4[++collision_row][collision_col];
		} else if (level_status == 4) {
			blocked = c_map5[collision_row][collision_col];
			blocked_top = c_map5[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map5[++collision_row][collision_col];
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
			blocked = c_map1[collision_row][collision_col];
			blocked_top = c_map1[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map1[++collision_row][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];
			blocked_top = 0;//c_map2[--collision_row][collision_col];
			++collision_row;
			blocked_bot = 0;//c_map2[++collision_row][collision_col];
		} else if (level_status == 2) {
			blocked = c_map3[collision_row][collision_col];
			blocked_top = c_map3[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map3[++collision_row][collision_col];
		} else if (level_status == 3) {
			blocked = c_map4[collision_row][collision_col];
			blocked_top = c_map4[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map4[++collision_row][collision_col];
		} else if (level_status == 4) {
			blocked = c_map5[collision_row][collision_col];
			blocked_top = c_map5[--collision_row][collision_col];
			++collision_row;
			blocked_bot = c_map5[++collision_row][collision_col];
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
			blocked = c_map1[collision_row][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];
		} else if (level_status == 2) {
			blocked = c_map3[collision_row][collision_col];
		} else if (level_status == 3) {
			blocked = c_map4[collision_row][collision_col];
		} else if (level_status == 4) {
			blocked = c_map5[collision_row][collision_col];
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
			blocked = c_map1[collision_row][collision_col];
		} else if (level_status == 1) {
			blocked = c_map2[collision_row][collision_col];
		} else if (level_status == 2) {
			blocked = c_map3[collision_row][collision_col];
		} else if (level_status == 3) {
			blocked = c_map4[collision_row][collision_col];
		} else if (level_status == 4) {
			blocked = c_map5[collision_row][collision_col];
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
	level_switch = 0;
}

void level_intro(void) {
	PPU_ADDRESS = NAMETABLE0_HIGH + 0x01;
	PPU_ADDRESS = NAMETABLE0_LOW  + 0xac;
	PPU_DATA    = 'L';
	PPU_DATA    = 'e';
	PPU_DATA    = 'v';
	PPU_DATA    = 'e';
	PPU_DATA    = 'l';
	PPU_DATA    = ' ';
	PPU_DATA    = NUMBER_0 + level_status;
	if (level_status == 0) {
		reset_scroll();
		PPU_ADDRESS = NAMETABLE0_HIGH + 0x02;
		PPU_ADDRESS = NAMETABLE0_LOW  + 0xa5;
		PPU_DATA = 'P';
		PPU_DATA = 'r';
		PPU_DATA = 'e';
		PPU_DATA = 's';
		PPU_DATA = 's';
		PPU_DATA = ' ';
		PPU_DATA = 'A';
		PPU_DATA = ' ';
		PPU_DATA = 't';
		PPU_DATA = 'o';
		PPU_DATA = ' ';
		PPU_DATA = 'E';
		PPU_DATA = 'n';
		PPU_DATA = 't';
		PPU_DATA = 'e';
		PPU_DATA = 'r';
		PPU_DATA = ' ';
		PPU_DATA = 'D';
		PPU_DATA = 'o';
		PPU_DATA = 'o';
		PPU_DATA = 'r';
		PPU_DATA = 's';
	}
	reset_scroll();
}

void clear_nametable(void) {
	PPU_ADDRESS = NAMETABLE0_HIGH;
	PPU_ADDRESS = NAMETABLE0_LOW;
	UnRLE(empty);
	reset_scroll();
}

void level3_tele_logic(void) {
	if (InputPort1 & BUTTON_A) {
		if (player_tl.x >= 184 - PLAYER_WIDTH && player_tl.x <= 184 + PLAYER_WIDTH &&
			player_tl.y >= 72 - PLAYER_HEIGHT && player_tl.y <= 72 + PLAYER_HEIGHT) {
			if (level_switch == 0 || level_switch == 2) {
				start_x = 48;
				start_y = 88;
				level_switch = 2;
				set_player();
			} else if (level_switch == 1) {
				game_state == Level_Passed;
				level_switch = 0;
			}
		}
		if (player_tl.x >= 184 - PLAYER_WIDTH && player_tl.x <= 184 + PLAYER_WIDTH &&
			player_tl.y >= 104 - PLAYER_HEIGHT && player_tl.y <= 104 + PLAYER_HEIGHT) {
			if (level_switch == 0) {
				start_x = 48;
				start_y = 88;
				level_switch = 1;
				set_player();
			} else if (level_switch == 1) {
				start_x = 48;
				start_y = 88;
				level_switch = 2;
				set_player();
			} else if (level_switch == 2) {
				game_state = Level_Passed;
				level_switch = 0;
			}
		}
	}
}

void level4_tele_logic(void) {
	if (InputPort1 & BUTTON_A) {
		if (player_tl.x >= 184 - PLAYER_WIDTH && player_tl.x <= 184 + PLAYER_WIDTH &&
			player_tl.y >= 72 - PLAYER_HEIGHT && player_tl.y <= 72 + PLAYER_HEIGHT) {
			if (level_switch == 0 || level_switch == 2) {
				start_x = 48;
				start_y = 88;
				level_switch = 1;
				set_player();
			} else if (level_switch == 1) {
				game_state == Level_Passed;
			}
		}
		if (player_tl.x >= 180 - PLAYER_WIDTH && player_tl.x <= 192 + PLAYER_WIDTH &&
			player_tl.y >= 104 - PLAYER_HEIGHT && player_tl.y <= 104 + PLAYER_HEIGHT) {
			if (level_switch == 0 || level_switch == 1) {
				start_x = 48;
				start_y = 88;
				level_switch = 2;
				set_player();
			} else if (level_switch == 2) {
				game_state = Level_Passed;
			}
		}
	}
}

void level5_tele_logic(void) {
	if (InputPort1 & BUTTON_A) {
		if (player_tl.x >= 64 - PLAYER_WIDTH && player_tl.x <= 64 + PLAYER_WIDTH &&
			player_tl.y >= 40 - PLAYER_HEIGHT && player_tl.y <= 40 + PLAYER_HEIGHT) {
			// top-left-top
			if (level_switch == 0) {
				start_x = 184; // send to top-right
				start_y = 56;
				set_player();
				level_switch = 0;
			}
		}
		if (player_tl.x >= 24 - PLAYER_WIDTH && player_tl.x <= 24 + PLAYER_WIDTH &&
			player_tl.y >= 72 - PLAYER_HEIGHT && player_tl.y <= 72 + PLAYER_HEIGHT) {
			// top-left-left
			if (level_switch == 0) {
				start_x = 184; // send to top-right
				start_y = 56;
				set_player();
				level_switch = 2;				
			}
		}
		if (player_tl.x >= 64 - PLAYER_WIDTH && player_tl.x <= 64 + PLAYER_WIDTH &&
			player_tl.y >= 72 - PLAYER_HEIGHT && player_tl.y <= 72 + PLAYER_HEIGHT) {
			// top-left-bottom
			if (level_switch == 0) {
				start_x = 120; // send to bottom
				start_y = 152;
				set_player();
				level_switch = 2;		
			}
		}
		if (player_tl.x >= 120 && player_tl.x <= 120 + PLAYER_WIDTH &&
			player_tl.y >= 104 - PLAYER_HEIGHT && player_tl.y <= 104 + PLAYER_HEIGHT) {
			// middle
			if (level_switch == 3) {
				game_state = Level_Passed;
				level_switch = 0;
			} else {
				start_x = 24; // send top-left
				start_y = 40;
				set_player();
				level_switch = 0;
			}
		}
		if (player_tl.x >= 224 - PLAYER_WIDTH && player_tl.x <= 224 + PLAYER_WIDTH &&
			player_tl.y >= 40 - PLAYER_HEIGHT && player_tl.y <= 40 + PLAYER_HEIGHT) {
			// top-right-top
			if (level_switch == 0) {
				start_x = 112; // send to middle
				start_y = 104;
				set_player();
				level_switch = 1;
			} else if (level_switch == 2) {
				start_x = 112; // send to middle
				start_y = 104;
				set_player();
				level_switch = 3;
			}
		}	
		if (player_tl.x >= 224 - PLAYER_WIDTH && player_tl.x <= 224 + PLAYER_WIDTH &&
			player_tl.y >= 72 - PLAYER_HEIGHT && player_tl.y <= 72 + PLAYER_HEIGHT) {
			// top-right-bottom
			if (level_switch == 2) {
				start_x = 112; // send to middle
				start_y = 104;
				set_player();
				level_switch = 1;			
			} else if (level_switch == 0) {
				start_x = 120; // send to bottom
				start_y = 152;
				set_player();
				level_switch = 1;						
			}
		}	
		if (player_tl.x >= 40 - PLAYER_WIDTH && player_tl.x <= 40 + PLAYER_WIDTH &&
			player_tl.y >= 200 - PLAYER_HEIGHT && player_tl.y <= 200 + PLAYER_HEIGHT) {
			// bottom-left
			if (level_switch == 1) {
				start_x = 24; // send top-left
				start_y = 40;
				set_player();
				level_switch = 0;
			} else if (level_switch == 2) {
				start_x = 24; // send top-left
				start_y = 40;
				set_player();
				level_switch = 0;
			}
		}	
		if (player_tl.x >= 208 - PLAYER_WIDTH && player_tl.x <= 208 + PLAYER_WIDTH &&
			player_tl.y >= 200 - PLAYER_HEIGHT && player_tl.y <= 200 + PLAYER_HEIGHT) {
			// bottom-right
			if (level_switch == 1) {
				start_x = 112; // send to middle
				start_y = 104;
				set_player();
				level_switch = 3;
			} else if (level_switch == 2) {
				start_x = 24; // send top-left
				start_y = 40;
				set_player();
				level_switch = 0;				
			}
		}	

	}
}

void draw_title(void) {
	PPU_ADDRESS = NAMETABLE0_HIGH + 0x01;
	PPU_ADDRESS = NAMETABLE0_LOW  + 0xa7;
	PPU_DATA    = 'B';
	PPU_DATA    = 'E';
	PPU_DATA    = 'R';
	PPU_DATA    = 'T';
	PPU_DATA    = '\'';
	PPU_DATA    = 'S';
	PPU_DATA    = ' ';
	PPU_DATA    = ' ';
	PPU_DATA    = 'A';
	PPU_DATA    = 'D';
	PPU_DATA    = 'V';
	PPU_DATA    = 'E';
	PPU_DATA    = 'N';
	PPU_DATA    = 'T';
	PPU_DATA    = 'U';
	PPU_DATA    = 'R';
	PPU_DATA    = 'E';

	PPU_ADDRESS = NAMETABLE0_HIGH + 0x06; //05
	PPU_ADDRESS = NAMETABLE0_LOW  + 0xa6;

	PPU_DATA    = 'P';
	PPU_DATA    = 'r';
	PPU_DATA    = 'e';
	PPU_DATA    = 's';
	PPU_DATA    = 's';
	PPU_DATA    = ' ';
	PPU_DATA    = 'S';
	PPU_DATA    = 't';
	PPU_DATA    = 'a';
	PPU_DATA    = 'r';
	PPU_DATA    = 't';
	PPU_DATA    = ' ';
	PPU_DATA    = 't';
	PPU_DATA    = 'o';
	PPU_DATA    = ' ';
	PPU_DATA    = 'P';
	PPU_DATA    = 'l';
	PPU_DATA    = 'a';
	PPU_DATA    = 'y';
	PPU_DATA    = ' ';

	reset_scroll();
}

void draw_end(void) {
	// draw empty background
	screen_off();
	PPU_ADDRESS = NAMETABLE0_HIGH;
	PPU_ADDRESS = NAMETABLE0_LOW;
	UnRLE(end);
	Wait_Vblank();
	screen_on();
	reset_scroll();

	// draw their time
	PPU_ADDRESS = NAMETABLE0_HIGH + 0x01;
	PPU_ADDRESS = NAMETABLE0_LOW  + 0x8a;
	PPU_DATA = 'T';
	PPU_DATA = 'i';
	PPU_DATA = 'm';
	PPU_DATA = 'e';
	PPU_DATA = ' ';
	PPU_DATA = NUMBER_0 + time_min;
	PPU_DATA = 0x3a; // ':'
	PPU_DATA = NUMBER_0 + time_sec_high;
	PPU_DATA = NUMBER_0 + time_sec_low;
	reset_scroll();
}

