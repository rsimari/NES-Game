/*
	author: Robert Simari
	date: 2/11/18
	note: code draws from doug fraker @ https://nesdoug.com/
*/
#include "DEFINE.c"
#include "GLOBAL.c"

// include background files
#include "BG/1.h"
#include "BG/2.h"
#include "BG/3.h"
#include "BG/4.h"

// pointers to each background file
const unsigned char* const All_Backgrounds[] = {n1, n2, n3, n4};

// include background collision data
#include "BG/1.csv"
#include "BG/2.csv"
#include "BG/3.csv"
#include "BG/4.csv"

const unsigned char* const All_Collision_Maps[] = {c1, c2, c3, c4};

// refers to asm4c.s code which decompresses the RLE background file
void __fastcall__ UnRLE(const unsigned char* data);
// refers to asm4c.s code which makes code wait for v-blank to write background
void Wait_Vblank(void);
// refers to asm4c.s code which reads from the controller
void Get_Input(void);
// memcpy for collision tables
void __fastcall__ memcpy(void* dest, const void* src, int count);

// import utility functions
#include "sprite_utils.c"
#include "screen_utils.c"
#include "input_utils.c"

void main (void) {
	screen_off();
	X_POS = 0x7f; // starting pos for sprite
	Y_POS = 0x77; 
	load_palette();
	reset_scroll();
	screen_on();

	while (1) {
		// wait for NMI interrupt
		while (NMI_flag == 0);

		if (new_background != 0) {
			draw_background();
			new_background = 0;
		}

		Get_Input();
		movement_logic();
		check_start();
		update_sprites();

		NMI_flag = 0;
	}
}



