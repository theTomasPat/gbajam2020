#include <string.h>
#include "gba.h"
#include "mgba.h"

// 0 if no error
inline int mgba_open() {
	*REG_DEBUG_ENABLE = 0xC0DE;
	return *REG_DEBUG_ENABLE == 0xC0DE;
}

inline void mgba_close() {
	*REG_DEBUG_ENABLE = 0;
}

inline void mgba_printf(u32 level, char *str) {
	memcpy(REG_DEBUG_STRING, str, DEBUG_MSG_LEN);
	*REG_DEBUG_FLAGS = (level & 0x7) | 0x100;
}
