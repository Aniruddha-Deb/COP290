#include "entity.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <complex>

#include "exceptions.hpp"

entity::entity(point screen_pos, point screen_dim, SDL_Texture* tex, point tex_pos, point tex_dim)
    : tex{tex} {
    src_frame.x = tex_pos.first;
    src_frame.y = tex_pos.second;
    src_frame.w = tex_dim.first;
    src_frame.h = tex_dim.second;
    src_ptr = &src_frame;

    dst_frame.x = screen_pos.first;
    dst_frame.y = screen_pos.second;
    dst_frame.w = screen_dim.first;
    dst_frame.h = screen_dim.second;
    dst_ptr = &dst_frame;
}

entity::entity(SDL_Texture* tex, point tex_pos, point tex_dim) : tex{tex} {
    src_frame.x = tex_pos.first;
    src_frame.y = tex_pos.second;
    src_frame.w = tex_dim.first;
    src_frame.h = tex_dim.second;
    src_ptr = &src_frame;

    dst_ptr = nullptr;
}

entity::~entity() {
    if (tex != nullptr) SDL_DestroyTexture(tex);
    dst_ptr = src_ptr = nullptr;
    tex = nullptr;
}

SDL_Texture* entity::get_texture() { return tex; }
SDL_Rect* entity::get_src_ptr() { return src_ptr; }
SDL_Rect* entity::get_dst_ptr() { return dst_ptr; }
