#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// #include "entity.hpp"
#include "exceptions.hpp"
#include "globals.hpp"

class render_window {
   public:
    render_window(const std::string& title, int width, int height);
    ~render_window();

    SDL_Texture* load_texture(const std::string& path);
    static TTF_Font* load_font(const std::string& path, int ptsize);

    void display();
    void clear();
    // void render(entity&);
    void scale(int);
    void render_text(TTF_Font* font, const std::string& text, int x, int y,
                     SDL_Color col = CL_WHITE);
    void render_text_left(TTF_Font* font, const std::string& text, int x, int y,
                          SDL_Color col = CL_WHITE);
    void render_heading(TTF_Font* font, const std::string& heading, int x, int y);

    //  private:
    SDL_Window* win;
    SDL_Renderer* ren;
};
