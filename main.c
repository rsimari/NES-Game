/*
	author: Robert Simari
	date: 2/11/18
	note: code draws from doug fraker @ https://nesdoug.com/
*/

/* Defines references to PPU registers that are commonly used */
#define PPU_CTRL    *((unsigned char*)0x2000)
#define PPU_MASK    *((unsigned char*)0x2001)
#define PPU_STATUS  *((unsigned char*)0x2002)
#define SCROLL      *((unsigned char*)0x2005)
#define PPU_ADDR    *((unsigned char*)0x2006) // where to write to
#define PPU_DATA    *((unsigned char*)0x2007) // what to write

// NES controller definitions
#define RIGHT		0x01
#define LEFT		0x02
#define DOWN		0x04
#define UP			0x08
#define START		0x10
#define SELECT		0x20
#define B_BUTTON	0x40
#define A_BUTTON	0x80

#pragma bss-name(push, "ZEROPAGE")
unsigned char NMI_flag; // for when NMI interrupt is activate for writing during V-blank
unsigned char Frame_Count; // counts number frames that have passed since start
unsigned char index;
unsigned char index4;
unsigned char Text_Position; // position of next letter to write
unsigned char X_POS; // x coord of sprite
unsigned char Y_POS; // y coord of sprite
unsigned char move;
unsigned char move4;
unsigned char move_count;
unsigned char state;
unsigned char state4;
// used and set in asm4c.s
unsigned char joypad1;
unsigned char joypad1old;
unsigned char joypad1test; 
unsigned char joypad2; 
unsigned char joypad2old;
unsigned char joypad2test;

#pragma bss-name(push, "OAM") // initializes segment OAM for compiler
unsigned char SPRITES[256];

const unsigned char TEXT[]={"Dont tell me I cant pbui"};

const unsigned char PALETTE[] = { // loaded at 0x23c0 for color palettes
	0x19, 0, 0, 0,  
	0, 0, 0, 0,  
	0, 0, 0, 0,  
	0, 0, 0, 0,
	0x19, 0x37, 0x24, 1,  
	0, 0, 0, 0,  
	0, 0, 0, 0,  
	0, 0, 0, 0
};

const unsigned char ATTR_TABLE[] = { // directs each quadrant of screen to which pallete to use
	/*
		bits = 44332211
		screen = 
			1, 2
			3, 4
		refer to table at bottom @ https://nesdoug.com/2015/11/19/5-a-little-color/ for info
		Note: this will grow to 0x23c0 - 0x23ff for entire screen
	*/
	0x44, // top left - 0100 0100
	0xbb, // top right - 1011 1011
	0x44, // bottom left - 0100 0100
	0xbb // bottom right - 1011 1011
};

const unsigned char MetaSprite_Y[] = { 0, 0, 8, 8 }; // relative y coords
const unsigned char MetaSprite_X[] = { 0, 8, 0, 8 }; // relative x coords
const unsigned char MetaSprite_Attr[] = { 0, 0, 0, 0 }; // used for flipping and palette

const unsigned char MetaSprite_Tile[] = { //tile numbers
	2, 3, 0x12, 0x13, //right
	0, 1, 0x10, 0x11, //down
	6, 7, 0x16, 0x17, //left
	4, 5, 0x14, 0x15  //up
}; 
// const unsigned char MetaSprite_Tile[] = {0, 1, 0x10, 0x11}; // tile numbers

#define Going_Right 0
#define Going_Down  1
#define Going_Left  2
#define Going_Up    3

#include "screen_utils.c"

void Get_Input(void); // refers to asm4c.s code which reads from the controller
void movement_logic(void);

void main (void) {
	screen_off();
	X_POS = 0x7f; // starting pos for sprite
	Y_POS = 0x77; 
	load_palette();
	screen_on();

	while (1) {
		// wait for NMI interrupt
		while (NMI_flag == 0);

		Get_Input();
		movement_logic();
		update_sprites();

		NMI_flag = 0;
	}
}

void movement_logic(void) {
	if ((joypad1 & RIGHT) != 0) {
		state = Going_Right;
		++X_POS;
	}
	if ((joypad1 & LEFT) != 0) {
		state = Going_Left;
		--X_POS;
	}
	if ((joypad1 & DOWN) != 0) {
		state = Going_Down;
		++Y_POS;
	}
	if ((joypad1 & UP) != 0) {
		state = Going_Up;
		--Y_POS;
	}
}


