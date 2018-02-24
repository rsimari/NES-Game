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

// addresses of background palettes
#define BG_PALETTE_HIGH  0x3f // universal background color
#define BG_PALETTE_LOW   0x00
#define BG_PALETTE0_HIGH 0x3f // background palette 0
#define BG_PALETTE0_LOW  0x01
#define BG_PALETTE1_HIGH 0x3f // background palette 1
#define BG_PALETTE1_LOW  0x05
#define BG_PALETTE2_HIGH 0x3f // background palette 2
#define BG_PALETTE2_LOW  0x09
#define BG_PALETTE3_HIGH 0x3f // background palette 3
#define BG_PALETTE3_LOW  0x0d

// sprite palettes
#define SP_PALETTE0_HIGH 0x3f
#define SP_PALETTE0_LOW  0x11
#define SP_PALETTE1_HIGH 0x3f
#define SP_PALETTE1_LOW  0x15
#define SP_PALETTE2_HIGH 0x3f
#define SP_PALETTE2_LOW  0x19
#define SP_PALETTE3_HIGH 0x3f
#define SP_PALETTE3_LOW  0x1d

// address of start of nametable 0-3, $2000-$2c00
#define NAMETABLE0_HIGH 0x20
#define NAMETABLE0_LOW  0x00
#define NAMETABLE1_HIGH 0x24
#define NAMETABLE1_LOW  0x00
#define NAMETABLE2_HIGH 0x28
#define NAMETABLE2_LOW  0x00
#define NAMETABLE3_HIGH 0x2c
#define NAMETABLE3_LOW  0x00

// helpful color definitions
#define COLOR_BLACK 0x0f
#define COLOR_WHITE 0x20
#define COLOR_BLUE  0x12
#define COLOR_GREEN 0x1a
#define COLOR_RED   0x17

// helpful screen dimension definitions
#define MIN_X 0
#define MAX_X 256
#define MIN_Y 0
#define MAX_Y 239

// .chr mappings
#define PLAYER_TILE   0x10
#define SPRITE_HEIGHT 0x10

// button addres mappings
#define BUTTON_RIGHT  0x01
#define BUTTON_LEFT   0x02
#define BUTTON_DOWN   0x04
#define BUTTON_UP     0x08
#define BUTTON_START  0x10
#define BUTTON_SELECT 0x20
#define BUTTON_B      0x40
#define BUTTON_A      0x80

// helpful C constants and variable types
#include <stddef.h>
#include <stdint.h>

extern uint8_t Frame_Number;
#pragma zpsym("Frame_Number");

extern uint8_t InputPort1;
#pragma zpsym("InputPort1");

extern uint8_t InputPort1Prev;
#pragma zpsym("InputPort1Prev");

extern uint8_t InputPort2;
#pragma zpsym("InputPort2");

extern uint8_t InputPort2Prev;
#pragma zpsym("InputPort2Prev");

// puts an iterator variable and others in the zeropage for fast access
#pragma bss-name(push, "ZEROPAGE")
size_t i;
#pragma bss-name(pop)

typedef struct sprite {
	unsigned char y;
	unsigned char tile_index;
	unsigned char attributes;
	unsigned char x;
} sprite_t;

// put sprite in OAM memory segment 
#pragma bss-name(push, "OAM")
sprite_t player;
#pragma bss-name(pop)

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

// functions from reset.s
void WaitFrame(void);

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

