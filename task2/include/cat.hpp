#pragma once

#include <SDL2/SDL.h>

#include <string>

#include "globals.hpp"
#include "render_window.hpp"

struct cat {
    int pos_x, pos_y, sprite, img;

    cat() : pos_x{0}, pos_y{0}, sprite{0}, img{0} {}
    cat(int x, int y, int s, int i) : pos_x{x}, pos_y{y}, sprite{s}, img{i} {}

    void render(const render_window& win, const SDL_Rect& camera, SDL_Texture* cat_sprite);

    std::string serialize();
    static cat deserialize(std::string);
};

