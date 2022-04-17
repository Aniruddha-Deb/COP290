#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

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
    int id;
    int character;

    player() : pos_x{0}, pos_y{0}, dir{DIR_D}, moving{false}, iter{0}, id{-1} {}
    player(int x, int y) : pos_x{x}, pos_y{y}, dir{DIR_D}, moving{false}, iter{0}, id{-1} {}

    void update_sprite(const int incr);
    void update_state(const Uint8* state, int clk);
    SDL_Rect get_camera();
    void render(render_window& win, int character, const SDL_Rect& camera,
                SDL_Texture* player_sprite);

    std::string serialize();
    static player deserialize(std::string);
};

