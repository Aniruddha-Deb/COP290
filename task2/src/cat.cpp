#include "cat.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cassert>
#include <sstream>
#include <string>

#include "globals.hpp"
#include "render_window.hpp"

void cat::render(const render_window& win, const SDL_Rect& camera, SDL_Texture* cat_sprite) {
    SDL_Rect src = {sprite * T, 0, T, T};
    SDL_Rect dst = {pos_x * T - camera.x, pos_y * T - camera.y, T, T};
    SDL_RenderCopy(win.ren, cat_sprite, &src, &dst);
}

std::string cat::serialize() {
    std::stringstream ss;
    ss << "C " << pos_x << ' ' << pos_y << ' ' << sprite << ' ' << img;
    return ss.str();
}

cat cat::deserialize(std::string s) {
    std::stringstream ss;
    ss << s;

    char ch;
    ss >> ch;
    assert(ch == 'C');

    cat p;
    ss >> p.pos_x >> p.pos_y >> p.sprite >> p.img;
    return p;
}
