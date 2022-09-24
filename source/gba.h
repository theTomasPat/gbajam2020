#ifndef __GBA_DEFINES_
#define __GBA_DEFINES_

#include <stdint.h>

typedef  uint8_t  u8;
typedef   int8_t  s8;
typedef uint16_t u16;
typedef  int16_t s16;
typedef uint32_t u32;
typedef  int32_t s32;

#define RGB(r,g,b) (uint16_t)((r << 0) + (g << 5) + (b << 10))

#define VCOUNT_MEM   ((volatile uint16_t *)0x04000006)
#define DISPSTAT_MEM ((volatile uint16_t *)0x04000004)
#define VRAM_MEM ((uint16_t *)0x06000000)
#define OAM_MEM  ((uint16_t *)0x07000000)

#define BGPAL_MEM  ((uint16_t *)0x05000000)
#define OBJPAL_MEM ((uint16_t *)0x05000200)


// Display Control
#define REG_DISPCNT ((uint16_t *)0x04000000)

// Display Control, shift amounts
#define DISPCNT_BGMODE_SHIFT 0
#define DISPCNT_CGBMODE_SHIFT 3
#define DISPCNT_FRAMESELECT_SHIFT 4
#define DISPCNT_HBLANKFREE_SHIFT 5
#define DISPCNT_OBJMAPPING_SHIFT 6
#define DISPCNT_FORCEDBLANK_SHIFT 7
#define DISPCNT_BG0FLAG_SHIFT 8
#define DISPCNT_BG1FLAG_SHIFT 9
#define DISPCNT_BG2FLAG_SHIFT 10
#define DISPCNT_BG3FLAG_SHIFT 11
#define DISPCNT_OBJFLAG_SHIFT 12
#define DISPCNT_WIN0FLAG_SHIFT 13
#define DISPCNT_WIN1FLAG_SHIFT 14
#define DISPCNT_OBJWINFLAG_SHIFT 15



// BG Control
#define BG0CNT ((uint16_t *)0x04000008)
#define BG1CNT ((uint16_t *)0x0400000A)
#define BG2CNT ((uint16_t *)0x0400000C)
#define BG3CNT ((uint16_t *)0x0400000E)

// BG Control, shift amounts
#define BGXCNT_PRIORITY 0
#define BGXCNT_CHARBASEBLOCK 2
#define BGXCNT_MOSAIC 6
#define BGXCNT_COLORMODE 7
#define BGXCNT_SCRNBASEBLOCK 8
#define BGXCNT_DISPAREAOVERFLOW 13
#define BGXCNT_SCREENSIZE 14


// OAM OBJ Control, shift amounts
#define ATTR0_ROTSCALEFLAG 8
#define ATTR0_DBLSIZE 9
#define ATTR0_DISABLE 9
#define ATTR0_OBJMODE 10
#define ATTR0_OBJMOSAIC 12
#define ATTR0_COLORMODE 13
#define ATTR0_OBJSHAPE 13

#define ATTR0_YCOORD_SHIFT 0
#define ATTR0_YCOORD_MASK 0xFF
#define ATTR0_YCOORD(n) ((n) << ATTR0_YCOORD_SHIFT)

#define ATTR1_ROTSCALEPARAM 9
#define ATTR1_FLIPHOR 12
#define ATTR1_FLIPVERT 13
#define ATTR1_OBJSIZE 14

#define ATTR1_XCOORD_SHIFT 0
#define ATTR1_XCOORD_MASK 0xFF
#define ATTR1_XCOORD(n) ((n) << ATTR1_XCOORD_SHIFT)

#define ATTR2_CHARNAME 0
#define ATTR2_PRIORITY 10
#define ATTR2_PALETTE 12


// Keypad
#define KEYINPUT (volatile u16 *)0x04000130;

// Keypad, shift amounts
#define KEYPAD_A (1 << 0)
#define KEYPAD_B (1 << 1)
#define KEYPAD_SEL (1 << 2)
#define KEYPAD_START (1 << 3)
#define KEYPAD_R (1 << 4)
#define KEYPAD_L (1 << 5)
#define KEYPAD_U (1 << 6)
#define KEYPAD_D (1 << 7)
#define KEYPAD_RS (1 << 8)
#define KEYPAD_LS (1 << 9)



#endif
