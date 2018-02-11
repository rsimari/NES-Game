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

unsigned char index;
unsigned char NMI_flag; // for when NMI interrupt is activate for writing during V-blank
unsigned char Frame_Count; // counts number frames that have passed since start
unsigned char Text_Position; // position of next letter to write

const unsigned char TEXT[]={"Dont tell me I cant pbui"};
const unsigned char PALETTE[]={ 0x1f, 0x00, 0x10, 0x20 }; //black, gray, lt gray, white

void screen_off(void);
void screen_on(void);
void load_palette(void);
void reset_scroll(void);
void load_text_increment(void);

void main (void) {
	screen_off();
	load_palette();

	screen_on();

	while (1) {
		// wait for NMI interrupt
		while (NMI_flag == 0);
		NMI_flag = 0;

		if (Frame_Count == 30) {
			load_text_increment();
			Frame_Count = 0;
		}
	}
}

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

void load_palette(void) {
	// load a color palette
	PPU_ADDR = 0x3f; // set write address in the PPU to 0x3f00
	PPU_ADDR = 0x00;
	// PPU_DATA increments automatically value set in 0x2000:2
	for (index = 0; index < sizeof(PALETTE); ++index){
		PPU_DATA = PALETTE[index]; // writes Palette data to 0x3f00, then increments
	}
	// because we wrote to PPU_DATA
	reset_scroll();
}

void reset_scroll(void) {
	// resets scroll registers that get messed up after writing to PPU_DATA register
	PPU_ADDR = 0;
	PPU_ADDR = 0;
	SCROLL = 0;
	SCROLL = 0;
}

void load_text_increment(void) {
	if (Text_Position < sizeof(TEXT)) {
		PPU_ADDR = 0x21; // location address on the screen to write
		PPU_ADDR = 0xc5 + Text_Position;
		PPU_DATA = TEXT[Text_Position];
		// always use ++x instead of x++, its faster
		++Text_Position;
	} else {
		// if we have written all letters go to beginning
		Text_Position = 0;
		PPU_ADDR = 0x21;
		PPU_ADDR = 0xc5;
		// clear all PPU_DATA registers to they dont get drawn in next frame
		for (index = 0; index < sizeof(TEXT); ++index) {
			PPU_DATA = 0;
		}
	}
	reset_scroll();
}