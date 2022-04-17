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
    std::cout << "Start server? ";
    std::string in;
    std::cin >> in;
    return in[0] == 'Y' || in[0] == 'y' || in[0] == '1';
}

bool inside(const SDL_Rect* r, int x, int y) {
    return x > r->x && x < r->x + r->w && y > r->y && y < r->y + r->h;
}

int main(int argc, char* argv[]) {
    //   SDLNet_Init();
    try {
        bool start_server = ask_server();
        gameStates g_state = GS_MMENU;

        render_window win("IITD Simulator", T * WIN_W * SCALE, T * WIN_H * SCALE);
        win.scale(SCALE);

        server_class S(start_server);  // start the server if start_server is true

        const Uint8* key_state = SDL_GetKeyboardState(nullptr);

        const auto map = win.load_texture("../assets/iitd_map.png");
        const auto player_sprite = win.load_texture("../assets/characters.png");
        player p(13 * T + T * WIN_W / 2,
                 78 * T + T * WIN_H / 2);  // change this to change start location

        SDL_Rect camera = p.get_camera();

        const auto m5x7 = win.load_font("../assets/m5x7.ttf", 16);
        const auto m5x7_l = win.load_font("../assets/m5x7.ttf", 32);
        const auto minecraftia = win.load_font("../assets/Minecraftia-Regular.ttf", 16);

        SDL_Event e;
        bool quit = false;

        int clk = 0;

        auto prev_loop_time = std::chrono::steady_clock::now().time_since_epoch();
        int tgt_loc = -1, cntdwn_timer = 3;
        std::pair<std::pair<int, int>, std::pair<int, int>> spawns;

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

                std::thread send_loop([&, sf]() {
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

        std::string input_ip = "";

        while (!quit) {
            clk = (clk + 1) % 4;

            {
                // update camera
                std::lock_guard<std::mutex> lock(player_mutex);
                // p.update_state(key_state, clk);
                camera = p.get_camera();
            }

            // render stuff
            win.clear();

            // map
            SDL_RenderCopy(win.ren, map, &camera, nullptr);

            // render other players
            {
                std::lock_guard<std::mutex> lock(others_mutex);
                for (auto [id, other_p] : others) other_p.render(win, camera, player_sprite);
            }
            // render me
            {
                std::lock_guard<std::mutex> lock(player_mutex);
                p.render(win, camera, player_sprite);
            }

            if (g_state == GS_MMENU) {
                win.render_heading(m5x7_l, "IITD Simulator", WIN_W * T / 2, T);
                win.render_text(m5x7, "Find a Game", WIN_W * T / 4, (WIN_H - 2) * T);
                win.render_text(m5x7, "Create a Game", 3 * WIN_W * T / 4, (WIN_H - 2) * T);
                // m5x7 doesn't have < and > glyphs, sad
                win.render_text(minecraftia, ">", (WIN_W / 2 + 1) * T + T / 2,
                                WIN_H * T / 2 + T / 2);
                win.render_text(minecraftia, "<", (WIN_W / 2 - 1) * T + T / 2,
                                WIN_H * T / 2 + T / 2);
            } else if (g_state == GS_WAIT) {
                win.render_text(m5x7, "IP: 127.0.0.1", 3 * T, T / 2);
                win.render_text(m5x7, "1/2", (WIN_W - 1) * T, T / 2);

                {
                    std::lock_guard<std::mutex> lock(player_mutex);
                    p.update_state(key_state, clk);
                }
                // free roam mode (without being able to see regions) for the server creator
                // TODO check if the number of people connected is 2/2
                // If so, enter countdown mode in both clients (send a message across)
            } else if (g_state == GS_COUNTDOWN) {
                if (cntdwn_timer == 0) {
                    // TODO server should send a packet informing both players of the
                    // target location to reach, and a spawn point for each of
                    // them (chosen equidistantly?)
                    tgt_loc = get_target_location();
                    spawns = get_spawn_points_for_location(tgt_loc);
                    std::cout << "(" << spawns.first.first << "," << spawns.first.second << "),("
                              << spawns.second.first << "," << spawns.second.second << ")"
                              << std::endl;
                    g_state = GS_CHASE;
                } else {
                    win.render_text(m5x7, "Starting in " + std::to_string(cntdwn_timer),
                                    WIN_W * T / 2, (WIN_H - 1) * T);
                    {
                        std::lock_guard<std::mutex> lock(player_mutex);
                        p.update_state(key_state, clk);
                    }
                }
                win.display();
            } else if (g_state == GS_FIND) {
                win.render_text(m5x7, "Enter IP address", WIN_W * T / 2, WIN_H * T / 4);
                SDL_RenderDrawLine(win.ren, (WIN_W - 4) * T / 2, WIN_H * T / 4 + 3 * T / 2,
                                   (WIN_W + 4) * T / 2, WIN_H * T / 4 + 3 * T / 2);
                if (input_ip.length() > 0) {
                    win.render_text(m5x7, input_ip.c_str(), WIN_W * T / 2, WIN_H * T / 4 + T);
                }
            } else if (g_state == GS_CHASE) {
                std::string prompt = str_regions[tgt_loc];
                prompt = "Get to " + prompt;
                win.render_text(m5x7, prompt.c_str(), WIN_W * T / 2, T);
                {
                    std::lock_guard<std::mutex> lock(player_mutex);
                    p.update_state(key_state, clk);
                }
                display_region(p, win, m5x7);
                // TODO check if either player reached target location (masala mix)
                // here. If one of them did, transition to GS_END
            } else if (g_state == GS_END) {
                // TODO check winner and print
                win.render_heading(m5x7_l, "Player 1 Won", WIN_W * T / 2, WIN_H * T / 2);
                win.render_text(m5x7, "Main Menu", WIN_W * T / 4, (WIN_H - 2) * T);
                win.render_text(m5x7, "Quit", 3 * WIN_W * T / 4, (WIN_H - 2) * T);
            }

            win.display();

            // get input
            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                    break;
                } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        int xm, ym;
                        SDL_GetMouseState(&xm, &ym);
                        if (inside(&BTN_LEFT, xm / 4, ym / 4)) {
                            if (g_state == GS_MMENU)
                                g_state = GS_FIND;
                            else if (g_state == GS_END) {
                                // reset game
                                g_state = GS_MMENU;
                                cntdwn_timer = 3;
                                camera = {13 * T, 78 * T, SCREEN_W, SCREEN_H};
                            }
                        } else if (inside(&BTN_RIGHT, xm / 4, ym / 4)) {
                            if (g_state == GS_MMENU)
                                g_state = GS_WAIT;
                            else if (g_state == GS_END) {
                                quit = true;
                                break;
                            }
                        }
                    }
                } else if (e.type == SDL_KEYUP) {
                    if (g_state == GS_MMENU) {
                        if (e.key.keysym.sym == SDLK_RIGHT) {
                            std::lock_guard<std::mutex> lock(player_mutex);
                            p.update_sprite(1);
                        } else if (e.key.keysym.sym == SDLK_LEFT) {
                            std::lock_guard<std::mutex> lock(player_mutex);
                            p.update_sprite(-1);
                        }
                    }
                } else if (e.type == SDL_KEYDOWN && g_state == GS_FIND) {
                    if ((e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) ||
                        e.key.keysym.sym == SDLK_PERIOD) {
                        if (input_ip.length() < 15) {
                            input_ip.push_back((char)e.key.keysym.sym);
                        }
                    }
                    if (e.key.keysym.sym == SDLK_BACKSPACE) {
                        if (input_ip.length() > 0) input_ip.erase(input_ip.length() - 1);
                    }
                    if (e.key.keysym.sym == SDLK_RETURN) {
                        // TODO connect to other server here and await countdown
                        // mode confirmation
                        g_state = GS_COUNTDOWN;
                    }
                }
            }

            if (clk == 0 && g_state == GS_COUNTDOWN) cntdwn_timer--;

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
