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

typedef struct sprite {
	uint8_t y;
	uint8_t tile_index;
	uint8_t attributes;
	uint8_t x;
} sprite_t;

// put sprite in OAM memory segment 
#pragma bss-name(push, "OAM")
sprite_t player;
#pragma bss-name(pop)

#endif