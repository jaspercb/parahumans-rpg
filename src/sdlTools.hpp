#pragma once

#include <SDL2/SDL.h>
#include "types.hpp"
#include "gfx/SDL2_gfxPrimitives.h"

namespace SDL_Colors {
	static const SDL_Color BLACK = {0, 0, 0, SDL_ALPHA_OPAQUE};
	static const SDL_Color LIGHTGREY = {63, 63, 63, SDL_ALPHA_OPAQUE};
	static const SDL_Color GREY = {127, 127, 127, SDL_ALPHA_OPAQUE};
	static const SDL_Color DARKGREY = {191, 191, 191, SDL_ALPHA_OPAQUE};
	static const SDL_Color WHITE = {255, 255, 255, SDL_ALPHA_OPAQUE};

	static const SDL_Color RED = {255, 0, 0, SDL_ALPHA_OPAQUE};
	static const SDL_Color GREEN = {0, 255, 0, SDL_ALPHA_OPAQUE};
	static const SDL_Color BLUE = {0, 0, 255, SDL_ALPHA_OPAQUE};

	static const SDL_Color LIGHTBLUE = {127, 127, 255, SDL_ALPHA_OPAQUE};

	static const SDL_Color YELLOW = {255, 204, 0, SDL_ALPHA_OPAQUE};
	static const SDL_Color ORANGE = {255, 104, 0, SDL_ALPHA_OPAQUE};
	static const SDL_Color PURPLE = {255, 0, 0, SDL_ALPHA_OPAQUE};

	static const SDL_Color TRANSPARENT = {0, 0, 0, SDL_ALPHA_TRANSPARENT};
};

Uint32 SDL_ColortoUint32(SDL_Color color);