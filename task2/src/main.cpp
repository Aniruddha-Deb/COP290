#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <exception>
#include <iostream>
#include <thread>

#include "exceptions.hpp"
#include "globals.hpp"
#include "location_tagging.hpp"
#include "player.hpp"
#include "render_window.hpp"

int main(int argc, char* argv[]) {
    try {
        render_window win("IITD Simulator", T * WIN_W * SCALE, T * WIN_H * SCALE);
        win.scale(SCALE);

        const Uint8* key_state = SDL_GetKeyboardState(nullptr);

        auto map = win.load_texture("../assets/iitd_map.png");
        SDL_Rect camera = {6 * T, 32 * T, SCREEN_W, SCREEN_H};

        auto player_sprite = win.load_texture("../assets/characters.png");
        player p(camera.x + T * WIN_W / 2,
                 camera.y + T * WIN_H / 2);  // change this to change start location

        auto m5x7 = win.load_font("../assets/m5x7.ttf", 16);

        SDL_Event e;
        bool quit = false;

        int clk = 0;

        auto start_time = std::chrono::steady_clock::now().time_since_epoch();

        while (!quit) {
            clk = (clk + 1) % 4;

            p.update_state(key_state);
            camera = p.get_camera();

            win.clear();
            SDL_RenderCopy(win.ren, map, &camera, nullptr);
            p.render(win, clk, 0, camera, player_sprite);
            display_region(p, win, m5x7);
            win.display();

            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                    break;
                }
            }

            {
                // makes it so that each loop takes 24 milliseconds to execute
                const auto cur_time = std::chrono::steady_clock::now().time_since_epoch();
                auto diff =
                    std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - start_time)
                        .count();
                static constexpr int loop_length = 24;
                auto wait_time = loop_length - diff;
                if (wait_time < 0) wait_time = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                start_time = cur_time;
            }
        }
    } catch (SDL_exception& e) {
        std::cerr << e.what() << std::endl;
    }

    (void)argc;
    (void)argv;
    return 0;
}
