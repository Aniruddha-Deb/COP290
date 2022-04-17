#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <utility>

#include "exceptions.hpp"

class entity {
   public:
    using point = std::pair<int, int>;

    entity(point screen_pos, point screen_dim, SDL_Texture*, point tex_pos, point tex_dim);
    entity(SDL_Texture*, point tex_pos, point tex_dim);
    ~entity();

    SDL_Texture* get_texture();
    SDL_Rect* get_src_ptr();
    SDL_Rect* get_dst_ptr();

   private:
    SDL_Rect *src_ptr, *dst_ptr;
    SDL_Rect src_frame, dst_frame;
    SDL_Texture* tex;
};

class player_entity : public entity {
   public:
    void handle_event(SDL_Event& e);

    int vel_x = 0, vel_y = 0; // just velocity
    int pos_x = 0, pos_y = 0; // relative to map
};