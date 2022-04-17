#pragma once

#include <SDL2/SDL_ttf.h>

#include <utility>

#include "player.hpp"
#include "render_window.hpp"

void display_region(const player& p, const render_window& win, TTF_Font* font);
int get_region(const player& p);
int get_target_location();
std::array<std::pair<int, int>, 2> get_spawn_points_for_location(int location);
