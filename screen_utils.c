void screen_off(void) {
	// turn off screen
	PPU_CTRL = 0; // set all bits to 0
	PPU_MASK = 0;
}

void screen_on(void) {
	// turn on screen
	PPU_CTRL = 0x90; // NMI on, bits = 1001 0000, this turns on NMI
	PPU_MASK = 0x1e; // screen on, bits = 0001 1110, this turns on all graphics
}

void reset_scroll(void) {
	// resets scroll registers that get messed up after writing to PPU_DATA register
	PPU_ADDR = 0;
	PPU_ADDR = 0;
	SCROLL = 0;
	SCROLL = 0;
}

void load_palette(void) {
	// load a color palette
	PPU_ADDR = 0x3f; // set write address in the PPU to 0x3f00
	PPU_ADDR = 0x00; // this is the address for background colors

	// PPU_DATA increments automatically value set in 0x2000:2
	for (index = 0; index < sizeof(PALETTE); ++index){
		PPU_DATA = PALETTE[index]; // writes Palette data to 0x3f00, then increments
	}
	// load attribute table
	// Note attribues can be loaded from 0x23c0 to 0x23ff, top left -> bottom right
	// PPU_ADDR = 0x23; // set write address to where we want to load our color palettes
	// PPU_ADDR = 0xda;
	// for (index = 0; index < sizeof(ATTR_TABLE); ++index) {
	// 	PPU_DATA = ATTR_TABLE[index];
	// }
	// because we wrote to PPU_DATA
	reset_scroll();
}

void load_text_increment(void) {
	if (Text_Position < sizeof(TEXT)) {
		PPU_ADDR = 0x21; // location address on the screen to write
		PPU_ADDR = 0xc4 + Text_Position;
		PPU_DATA = TEXT[Text_Position];
		// always use ++x instead of x++, its faster
		++Text_Position;
	} else {
		// if we have written all letters go to beginning
		Text_Position = 0;
		PPU_ADDR = 0x21;
		PPU_ADDR = 0xc4;
		// clear all PPU_DATA registers to they dont get drawn in next frame
		for (index = 0; index < sizeof(TEXT); ++index) {
			PPU_DATA = 0;
		}
	}
	reset_scroll();
}

void draw_background(void) {
	screen_off();
	PPU_ADDR = 0x20; // point PPU_ADDR to the beginning of the nametable/screen
	PPU_ADDR = 0x00;
	UnRLE(All_Backgrounds[which_BGD]); // uncompresses RLE file and sends it to PPU via PPU_DATA

	// load background collision map
	p_C_MAP = All_Collision_Maps[which_BGD];
	memcpy(C_MAP, p_C_MAP, 240);

	Wait_Vblank();
	screen_on();
	reset_scroll();
	++which_BGD;
	if (which_BGD == 4) // cycles between 4 backgrounds
		which_BGD = 0;
}

void check_four_sides(void) {	
	// 255 - # of blank pixels in sprite on certain side
	// the minus adjust for overflow because its unsigned
	if (X_POS < (255 - 3)) {
		X_POS_Left = X_POS + 4;
	} else {
		X_POS_Left = 255;
	}
	if (X_POS < (255 - 12)) {
		X_POS_Right = X_POS + 11;
	} else {
		X_POS_Right = 255;
	}
	Y_POS_Top = Y_POS + 9;
	if (Y_POS < (255 - 15)) {
		Y_POS_Bottom = Y_POS + 15;
	} else {
		Y_POS_Bottom = 255;
	}
}

void collision_up_down(void) {
	check_four_sides();

	// check down
	if ((joypad1 & DOWN) != 0) {
		// bottom right
		corner = ((X_POS_Right & 0xf0) >> 4) + (Y_POS_Bottom & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			Y_POS = (Y_POS & 0xf0);
		// bottom left
		corner = ((X_POS_Left & 0xf0) >> 4) + (Y_POS_Bottom & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			Y_POS = (Y_POS & 0xf0);
	  // check top
	} else if ((joypad1 & UP) != 0) {
		corner = ((X_POS_Right & 0xf0) >> 4) + (Y_POS_Top & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			Y_POS = (Y_POS & 0xf0) + 7;
		corner = ((X_POS_Left & 0xf0) >> 4) + (Y_POS_Top & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			Y_POS = (Y_POS & 0xf0) + 7;
	}
}

void collision_left_right(void) {
	check_four_sides();

	// check right
	if ((joypad1 & RIGHT) != 0) {
		// top right
		corner = ((X_POS_Right & 0xf0) >> 4) + (Y_POS_Top & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			X_POS = (X_POS & 0xf0) + 4;
		// bottom right
		corner = ((X_POS_Right & 0xf0) >> 4) + (Y_POS_Bottom & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			X_POS = (X_POS & 0xf0) + 4;
	  // check left
	} else if ((joypad1 & LEFT) != 0) {
		// top left
		corner = ((X_POS_Left & 0xf0) >> 4) + (Y_POS_Top & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			X_POS = (X_POS & 0xf0) + 12;
		// bottom left
		corner = ((X_POS_Left & 0xf0) >> 4) + (Y_POS_Bottom & 0xf0);
		if (C_MAP[corner] != 0) // collision occured
			X_POS = (X_POS & 0xf0) + 12;
	}
}


