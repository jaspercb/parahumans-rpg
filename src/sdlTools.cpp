#include "sdlTools.hpp"

Uint32 SDL_ColortoUint32(SDL_Color color) {
	#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		Uint32 int_color = color.a*0x1000000 | color.r*0x10000 | color.g*0x100 | color.b;
	#else
		Uint32 int_color = color.a*0x1000000 | color.r | color.g*0x100 | color.b*0x10000;
	#endif
	return int_color;
}
