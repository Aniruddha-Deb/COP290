#pragma once

#include <SDL2/SDL.h>

#include <array>

#include "globals.hpp"
#include "render_window.hpp"

struct player {
    static constexpr int W = T, H = T;
    static constexpr std::array<int, 4> conv = {0, 1, 0, 2};

    int pos_x, pos_y;
    directions dir;
    bool moving;
    int iter;

    player(int x, int y) : pos_x{x}, pos_y{y}, dir{DIR_D}, moving{false}, iter{0} {}

    void update_state(const Uint8* state);
    SDL_Rect get_camera();
    void render(render_window& win, int clk, int id, const SDL_Rect& camera, SDL_Texture* player_sprite);
};

