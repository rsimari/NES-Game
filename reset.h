/*
	links exported variables/functions from reset.s to our C code
*/

#ifndef RESET_H_
#define RESET_H_

#include <stdint.h>

extern uint8_t Frame_Number;
#pragma zpsym("Frame_Number"); // tells compiler that its a zeropage symbol thats already defined

extern uint8_t InputPort1;
#pragma zpsym("InputPort1");

extern uint8_t InputPort1Prev;
#pragma zpsym("InputPort1Prev");

extern uint8_t InputPort2;
#pragma zpsym("InputPort2");

extern uint8_t InputPort2Prev;
#pragma zpsym("InputPort2Prev");

void WaitFrame(void);

// puts an iterator variable and others in the zeropage for fast access
#pragma bss-name(push, "ZEROPAGE")
size_t i;
#pragma bss-name(pop)

// see https://wiki.nesdev.com/w/index.php/PPU_OAM
typedef struct sprite {
	uint8_t y; // y coord of sprite
	uint8_t tile_index; // index of tile in .chr file
	uint8_t attributes; // 7654 3210, 7: flip vert, 6: flip horiz
	                    // 5: 0 in front of bg
	                    // 4-2: nothing
	                    // 1-0: palette number starting at $3f10
	uint8_t x; // x coord of sprite
} sprite_t;

typedef struct meta_sprite {
	sprite_t sprites[4];
} meta_sprite_t;

// put sprite in OAM memory segment 
#pragma bss-name(push, "OAM")
sprite_t player_tl;
sprite_t player_tr;
sprite_t player_bl;
sprite_t player_br;
#pragma bss-name(pop)

#endif