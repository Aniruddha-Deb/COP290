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
    int id;
    directions dir;
    bool moving;
    int iter;

    player() : pos_x{0}, pos_y{0}, id{0}, dir{DIR_D}, moving{false}, iter{0} {}
    player(int x, int y) : pos_x{x}, pos_y{y}, id{0}, dir{DIR_D}, moving{false}, iter{0} {}

    void update_state(const Uint8* state);
    void update_sprite(const int incr);
    SDL_Rect get_camera();
    void render(render_window& win, int clk, const SDL_Rect& camera,
                SDL_Texture* player_sprite);

    std::string serialize();
    static player deserialize(std::string);
    void send_player(TCPsocket);
};

