#include <string.h>
#include "gba.h"
#include "mgba.h"

// 0 if no error
inline int mgba_open() {
	#if __DEBUG__
	*REG_DEBUG_ENABLE = 0xC0DE;
	return *REG_DEBUG_ENABLE == 0xC0DE;
	#endif
}

inline void mgba_close() {
	#if __DEBUG__
	*REG_DEBUG_ENABLE = 0;
	#endif
}

inline void mgba_printf(u32 level, char *str, u32 len) {
	#if __DEBUG__
	memcpy(REG_DEBUG_STRING, str, 0x100);
	*REG_DEBUG_FLAGS = (level & 0x7) | 0x100;
	#endif
}
