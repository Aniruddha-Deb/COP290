#pragma once

#include <SDL2/SDL_ttf.h>

#include "player.hpp"
#include "render_window.hpp"

void display_region(const player& p, const render_window& win, TTF_Font* font);
