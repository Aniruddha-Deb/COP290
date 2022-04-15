#include "render_window.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <iostream>

#include "exceptions.hpp"

render_window::render_window(const std::string& title, int width, int height)
    : win{nullptr}, ren{nullptr} {
    // Initialize renderer and window
    if (SDL_Init(SDL_INIT_VIDEO) != 0) throw SDL_exception("SDL_Init failed");

    win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width,
                           height, SDL_WINDOW_SHOWN);
    if (win == nullptr) throw SDL_exception("SDL_CreateWindow failed");

    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) throw SDL_exception("SDL_CreateRenderer failed");

    SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);

    // Initialize IMG
    if (IMG_Init(IMG_INIT_PNG) == 0) throw SDL_exception("IMG_Init failed");

    // Initialize TTF
    if (TTF_Init() != 0) throw SDL_exception("TTF_Init failed");
}

SDL_Texture* render_window::load_texture(const std::string& path) {
    SDL_Surface* surf = IMG_Load(path.c_str());
    if (surf == nullptr) throw SDL_exception("Texture at " + path + " doesn't exist");

    auto res = SDL_CreateTextureFromSurface(ren, surf);
    if (res == nullptr) throw SDL_exception("Texture couldn't be created from surface " + path);

    SDL_FreeSurface(surf);

    return res;
}

TTF_Font* render_window::load_font(const std::string& path, int ptsize) {
    auto res = TTF_OpenFont(path.c_str(), ptsize);
    if (res == nullptr) throw SDL_exception("Font at " + path + "couldn't be opened");
    return res;
}

/*
void render_window::render(entity& e) {
    if (SDL_RenderCopy(ren, e.get_texture(), e.get_src_ptr(), e.get_dst_ptr()) < 0)
        throw SDL_exception("SDL_RenderCopy failed");
}
*/

void render_window::scale(int val) {
    if (SDL_RenderSetScale(ren, (float)val, (float)val) < 0) throw SDL_exception("SDL_RenderSetScale failed");
}

void render_window::display() { SDL_RenderPresent(ren); }
void render_window::clear() {
    if (SDL_RenderClear(ren) < 0) throw SDL_exception("SDL_RenderClear failed");
}

render_window::~render_window() {
    IMG_Quit();

    if (ren != nullptr) SDL_DestroyRenderer(ren);
    if (win != nullptr) SDL_DestroyWindow(win);
    SDL_Quit();
}
