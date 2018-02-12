/* Defines references to PPU registers that are commonly used */
#define PPU_CTRL    *((unsigned char*)0x2000)
#define PPU_MASK    *((unsigned char*)0x2001)
#define PPU_STATUS  *((unsigned char*)0x2002)
#define OAM_ADDR    *((unsigned char*)0x2003)
#define SCROLL      *((unsigned char*)0x2005)
#define PPU_ADDR    *((unsigned char*)0x2006) // where to write to
#define PPU_DATA    *((unsigned char*)0x2007) // what to write
#define OAM_DMA     *((unsigned char*)0x4014)

// NES controller definitions
#define RIGHT		0x01
#define LEFT		0x02
#define DOWN		0x04
#define UP			0x08
#define START		0x10
#define SELECT		0x20
#define B_BUTTON	0x40
#define A_BUTTON	0x80

#define Going_Right 0
#define Going_Down  1
#define Going_Left  2
#define Going_Up    3