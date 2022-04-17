#pragma once

#include <SDL2/SDL_ttf.h>
#include <utility>

#include "player.hpp"
#include "render_window.hpp"

void display_region(const player& p, const render_window& win, TTF_Font* font);
int get_target_location();
std::pair<std::pair<int,int>,std::pair<int,int>> get_spawn_points_for_location(int location);
