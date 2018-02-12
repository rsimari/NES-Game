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
unsigned char new_background;
unsigned char which_BGD;
const unsigned char* p_C_MAP; // points to table of collisions
unsigned char X_POS_Right;
unsigned char X_POS_Left;
unsigned char Y_POS_Bottom;
unsigned char Y_POS_Top;
unsigned char corner;
// used and set in asm4c.s
unsigned char joypad1;
unsigned char joypad1old;
unsigned char joypad1test; 
unsigned char joypad2; 
unsigned char joypad2old;
unsigned char joypad2test;

// initializes segment OAM for compiler
#pragma bss-name(push, "OAM")
unsigned char SPRITES[256];

// initializes collision map in RAM: $300-3ff. TODO: change this to not RAM
#pragma bss-name(push, "MAP")
unsigned char C_MAP[256];

const unsigned char TEXT[]={"Dont tell me I cant pbui"};

const unsigned char PALETTE[] = { // loaded at 0x23c0 for color palettes
	0x1f, 0, 0x10, 0x20,  
	0, 0, 0, 0,  
	0, 0, 0, 0,  
	0, 0, 0, 0,
	0x1f, 0x37, 0x24, 1,  
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
