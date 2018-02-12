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
	PPU_ADDR = 0x23; // set write address to where we want to load our color palettes
	PPU_ADDR = 0xda;
	for (index = 0; index < sizeof(ATTR_TABLE); ++index) {
		PPU_DATA = ATTR_TABLE[index];
	}
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

void update_sprites(void) {
	index4 = 0;
	move4 = state << 2;
	for (index = 0; index < 4; ++index) {
		SPRITES[index4] = MetaSprite_Y[index] + Y_POS; // relative y + master y
		++index4;
		SPRITES[index4] = MetaSprite_Tile[index + move4]; // tile number switch
		++index4;
		SPRITES[index4] = MetaSprite_Attr[index]; // change attributes
		++index4;
		SPRITES[index4] = MetaSprite_X[index] + X_POS; // relative x + master x
		++index4;
	}
}
