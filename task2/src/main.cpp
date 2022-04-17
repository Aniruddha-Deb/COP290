#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <thread>
#include <unordered_map>

#include "exceptions.hpp"
#include "globals.hpp"
#include "location_tagging.hpp"
#include "player.hpp"
#include "render_window.hpp"
#include "server_class.hpp"

bool ask_server() {
    std::cout << "Start server?";
    std::string in;
    std::cin >> in;
    return in[0] == 'Y' || in[0] == 'y' || in[0] == '1';
}

int main(int argc, char* argv[]) {
    //   SDLNet_Init();
    try {
        bool start_server = ask_server();

        render_window win("IITD Simulator", T * WIN_W * SCALE, T * WIN_H * SCALE);
        win.scale(SCALE);

        server_class S(start_server);  // start the server if start_server is true

        const Uint8* key_state = SDL_GetKeyboardState(nullptr);

        const auto map = win.load_texture("../assets/iitd_map.png");
        const auto player_sprite = win.load_texture("../assets/characters.png");
        player p(6 * T + T * WIN_W / 2,
                 32 * T + T * WIN_H / 2);  // change this to change start location

        SDL_Rect camera = p.get_camera();

        const auto m5x7 = win.load_font("../assets/m5x7.ttf", 16);

        SDL_Event e;
        bool quit = false;

        int clk = 0;

        auto prev_loop_time = std::chrono::steady_clock::now().time_since_epoch();

        /*
         * have another client thread
         *
         * client thread:
         * - read p
         * - write others
         *
         * game loop only
         * - write p
         * - read others
         */

        std::mutex player_mutex, others_mutex;
        std::promise<void> client_exit_signal;

        std::unordered_map<int, player> others;

        std::thread client_thread(
            [&](std::future<void> exit_sig) {
                IPaddress server_ip;
                SDLNet_ResolveHost(&server_ip, "127.0.0.1", server_class::port);
                TCPsocket server = SDLNet_TCP_Open(&server_ip);

                {
                    std::lock_guard<std::mutex> lock(player_mutex);
                    server_class::send_packet(server, p.serialize());
                    // first connection

                    p = *server_class::get_player(server);
                }

                SDLNet_SocketSet server_set = SDLNet_AllocSocketSet(1);
                SDLNet_TCP_AddSocket(server_set, server);

                std::shared_future<void> sf(std::move(exit_sig));

                std::thread send_loop([&]() {
                    static constexpr auto send_loop_time = std::chrono::milliseconds(12);
                    while (sf.wait_for(send_loop_time) == std::future_status::timeout) {
                        std::lock_guard<std::mutex> lock(player_mutex);
                        server_class::send_packet(server, p.serialize());
                    }
                });

                int num_ready;
                while (sf.wait_for(std::chrono::microseconds(500)) == std::future_status::timeout) {
                    num_ready = SDLNet_CheckSockets(server_set, 0);
                    if (num_ready <= 0) continue;

                    if (SDLNet_SocketReady(server)) {
                        auto new_p = server_class::get_player(server);

                        std::lock_guard<std::mutex> lock(others_mutex);
                        others[new_p->id] = *new_p;
                    } else {
                        // server died
                        std::cout << "Server died :(\n";
                        break;
                    }
                }

                if (send_loop.joinable()) send_loop.join();

                SDLNet_TCP_Close(server);
            },
            client_exit_signal.get_future());

        while (!quit) {
            clk = (clk + 1) % 4;

            {
                std::lock_guard<std::mutex> lock(player_mutex);
                p.update_state(key_state, clk);
                camera = p.get_camera();
            }
            // p.send_player(server);

            // render stuff
            win.clear();

            // map
            SDL_RenderCopy(win.ren, map, &camera, nullptr);

            // other players
            {
                std::lock_guard<std::mutex> lock(others_mutex);
                for (auto [id, other_p] : others)
                    other_p.render(win, start_server, camera, player_sprite);
            }

            // me
            {
                std::lock_guard<std::mutex> lock(player_mutex);
                p.render(win, !start_server, camera, player_sprite);
                display_region(p, win, m5x7);
            }

            win.display();

            // get input
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                    break;
                }
            }

            {
                // makes it so that each loop takes 24 milliseconds to execute
                const auto cur_time = std::chrono::steady_clock::now().time_since_epoch();
                const auto diff =
                    std::chrono::duration_cast<std::chrono::milliseconds>(cur_time - prev_loop_time)
                        .count();
                static constexpr int loop_length = 24;
                auto wait_time = loop_length - diff;
                if (wait_time > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(wait_time));
                prev_loop_time = cur_time;
            }
        }

        client_exit_signal.set_value();
        if (client_thread.joinable()) client_thread.join();
    } catch (SDL_exception& e) {
        std::cerr << e.what() << std::endl;
    }

    (void)argc;
    (void)argv;
    return 0;
}
