#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <vector>

#include "cat.hpp"
#include "exceptions.hpp"
#include "globals.hpp"
#include "location_tagging.hpp"
#include "player.hpp"
#include "render_window.hpp"
#include "server_class.hpp"

bool inside(const SDL_Rect* r, int x, int y) {
    return x > r->x && x < r->x + r->w && y > r->y && y < r->y + r->h;
}

int main(int argc, char* argv[]) {
    try {
        render_window win("IITD Simulator", T * WIN_W * SCALE, T * WIN_H * SCALE);
        win.scale(SCALE);

        server_class S;  // object to gracefully handle server

        const Uint8* key_state = SDL_GetKeyboardState(nullptr);

        // load textures
        const auto map = win.load_texture("../assets/iitd_map.png");
        const auto player_sprite = win.load_texture("../assets/characters.png");
        const auto cat_sprite = win.load_texture("../assets/cats.png");
        const auto cat_imgs = [&]() {
            std::array<SDL_Texture*, cat_img_count> res;
            for (size_t i = 0; i < cat_img_count; ++i)
                res[i] = win.load_texture("../assets/cats/cat" + std::to_string(i) + ".png");
            return res;
        }();
        auto cat_img = cat_imgs[0];

        const auto m5x7 = win.load_font("../assets/m5x7.ttf", 16);
        const auto m5x7_l = win.load_font("../assets/m5x7.ttf", 32);
        const auto minecraftia = win.load_font("../assets/Minecraftia-Regular.ttf", 16);

        SDL_Event e;
        bool quit = false;
        int clk = 0;

        // all game state information here
        gameStates g_state = GS_MMENU, prev_state = GS_WAIT;
        bool change_character = true;

        player p(13 * T + T * WIN_W / 2,
                 78 * T + T * WIN_H / 2);  // change this to change start location
        std::unordered_map<int, player> others;

        SDL_Rect camera = p.get_camera();

        std::atomic<bool> initiate_chase_request = false;
        std::atomic<int> tgt_loc = -1, winner = -1, my_id = -1;

        auto prev_loop_time = std::chrono::steady_clock::now().time_since_epoch();
        auto countdown_start = prev_loop_time;

        std::string input_ip = "";

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

        std::mutex player_mutex, others_mutex, cats_mutex;
        std::promise<void> client_exit_signal;
        std::vector<cat> cats;

        const auto client_loop = [&](std::future<void> exit_sig, std::string address) {
            IPaddress server_ip;

            if (SDLNet_ResolveHost(&server_ip, address.c_str(), server_class::port) < 0) {
                std::cout << "Couldn't resolve host\n";
            }

            TCPsocket server = SDLNet_TCP_Open(&server_ip);

            {
                std::lock_guard<std::mutex> lock(player_mutex);
                server_class::send_packet(server, p.serialize());
                // first connection

                p = *server_class::get_player(server);
                my_id = p.id;
            }

            SDLNet_SocketSet server_set = SDLNet_AllocSocketSet(1);
            SDLNet_TCP_AddSocket(server_set, server);

            std::shared_future<void> sf(std::move(exit_sig));

            std::thread send_loop([&, sf]() {
                static constexpr auto send_loop_time = std::chrono::milliseconds(12);
                while (sf.wait_for(send_loop_time) == std::future_status::timeout) {
                    if (initiate_chase_request) {
                        server_class::send_packet(server, "I");
                        initiate_chase_request = false;
                    } else {
                        std::lock_guard<std::mutex> lock(player_mutex);
                        server_class::send_packet(server, p.serialize());
                    }
                }
            });

            int num_ready;
            std::string buf;
            while (sf.wait_for(std::chrono::microseconds(500)) == std::future_status::timeout) {
                num_ready = SDLNet_CheckSockets(server_set, 0);
                if (num_ready <= 0) continue;

                if (SDLNet_SocketReady(server)) {
                    auto next_packet = server_class::get_packet(server);

                    if (next_packet) {
                        buf = *next_packet;
                        if (buf[0] == 'P') {
                            auto new_p = player::deserialize(buf);
                            std::lock_guard<std::mutex> lock(others_mutex);
                            others[new_p.id] = std::move(new_p);
                        } else if (buf[0] == 'D') {
                            auto del_id = std::stoi(buf.substr(1));
                            std::lock_guard<std::mutex> lock(others_mutex);

                            // Player with ID del_id got disconnected

                            others.erase(del_id);
                        } else if (buf[0] == 'I') {
                            std::stringstream ss;
                            ss << buf;

                            char c;
                            ss >> c;

                            int tmp;
                            ss >> tmp;
                            tgt_loc = tmp;

                            for (int i = 0; i < 2; ++i) {
                                int id, x, y;
                                ss >> id >> x >> y;
                                if (my_id == id) {
                                    std::lock_guard<std::mutex> lock(player_mutex);
                                    p.pos_x = (x - 1) * T;
                                    p.pos_y = (y - 1) * T;
                                    server_class::send_packet(server, p.serialize());
                                    break;
                                    // others will update their position in the next server query
                                }
                            }
                        } else if (buf[0] == 'W') {
                            std::stringstream ss;
                            ss << buf;
                            char ch;
                            ss >> ch;
                            int tmp;
                            ss >> tmp;
                            winner = tmp;
                        } else if (buf[0] == 'C') {
                            const auto c = cat::deserialize(buf);

                            std::lock_guard<std::mutex> lock(cats_mutex);
                            cats.push_back(c);
                        }
                    } else {
                        std::cout << "Server died here\n";
                        std::cout << "Server died :(\n";
                        break;
                    }
                } else {
                    // server died
                    std::cout << "Server died :(\n";
                    break;
                }
            }

            if (send_loop.joinable()) send_loop.join();

            SDLNet_TCP_Close(server);
        };

        std::thread client_thread;

        auto connect_to_server = [&](std::string address) {
            client_exit_signal = std::promise<void>();
            client_thread = std::thread(client_loop, client_exit_signal.get_future(), address);
        };
        auto kill_client = [&]() {
            client_exit_signal.set_value();
            if (client_thread.joinable()) client_thread.join();
        };

        while (!quit) {
            clk = (clk + 1) % 4;

            {
                // update camera
                std::lock_guard<std::mutex> lock(player_mutex);
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

            // render cats
            {
                std::lock_guard<std::mutex> loc(cats_mutex);
                for (auto c : cats) c.render(win, camera, cat_sprite);
            }

            if (g_state != GS_MMENU) {
                // Mouseover help
                int xm, ym;
                SDL_GetMouseState(&xm, &ym);
                SDL_Rect HELP_RECT{0, 0, T + T / 4, T + T / 4};

                if (inside(&HELP_RECT, xm / SCALE, ym / SCALE)) {
                    SDL_SetRenderDrawColor(win.ren, 0x00, 0x00, 0x00, 0x7F);
                    SDL_Rect background = {T / 4, T / 4, 8 * T + T / 4 + T / 4, 4 * T};
                    SDL_RenderFillRect(win.ren, &background);
                    SDL_SetRenderDrawColor(win.ren, 0xFF, 0xFF, 0xFF, 0xFF);

                    win.render_text_left(m5x7, "[s]tart the race", T / 2, T / 4);
                    win.render_text_left(m5x7, "[enter] to pet the cat", T / 2, T / 4 + 3 * T / 4);
                    win.render_text_left(m5x7, "[space] to start running", T / 2,
                                         T / 4 + 2 * (3 * T / 4));
                    win.render_text_left(m5x7, "[c]hange character", T / 2,
                                         T / 4 + 3 * (3 * T / 4));
                    win.render_text_left(m5x7, "[r]estart", T / 2, T / 4 + 4 * (3 * T / 4));
                } else {
                    win.render_heading(m5x7, "?", T / 2, T / 2);
                }
            }

            if (change_character) {
                std::lock_guard<std::mutex> lock(player_mutex);
                win.render_text(minecraftia, "<", (p.pos_x - camera.x) - T / 2,
                                (p.pos_y - camera.y) + T / 2);
                win.render_text(minecraftia, ">", (p.pos_x - camera.x) + 3 * T / 2,
                                (p.pos_y - camera.y) + T / 2);
            }

            if (g_state == GS_MMENU) {
                change_character = true;

                win.render_heading(m5x7_l, "IITD Simulator", WIN_W * T / 2, T);
                win.render_text(m5x7, "Find a Game", WIN_W * T / 4, (WIN_H - 2) * T);
                win.render_text(m5x7, "Create a Game", 3 * WIN_W * T / 4, (WIN_H - 2) * T);
                // m5x7 doesn't have < and > glyphs, sad
            } else if (g_state == GS_WAIT) {
                // win.render_text(m5x7, "IP: 127.0.0.1", 3 * T, T / 2);
                win.render_text(m5x7, std::to_string(1 + others.size()) + "/2", (WIN_W - 1) * T,
                                T / 2);

                if (!change_character) {
                    std::lock_guard<std::mutex> lock(player_mutex);
                    p.update_state(key_state, clk);
                }

                // free roam mode (without being able to see regions) for the server creator
                // TODO check if the number of people connected is 2/2
                // If so, enter countdown mode in both clients (send a message across)
                if (tgt_loc >= 0) {
                    countdown_start = std::chrono::steady_clock::now().time_since_epoch();

                    g_state = GS_COUNTDOWN;
                }
            } else if (g_state == GS_COUNTDOWN) {
                const auto cur_time = std::chrono::steady_clock::now().time_since_epoch();
                const auto cntdwn_timer =
                    3 - std::chrono::duration_cast<std::chrono::seconds>(cur_time - countdown_start)
                            .count();
                if (cntdwn_timer <= 0) {
                    // TODO server should send a packet informing both players of the
                    // target location to reach, and a spawn point for each of
                    // them (chosen equidistantly?)

                    /*
                    tgt_loc = get_target_location();
                    spawns = get_spawn_points_for_location(tgt_loc);
                    std::cout << "(" << spawns.first.first << "," << spawns.first.second << "),("
                              << spawns.second.first << "," << spawns.second.second << ")"
                              << std::endl;
                              */
                    g_state = GS_CHASE;
                } else {
                    win.render_text(m5x7, "Starting in " + std::to_string(cntdwn_timer),
                                    WIN_W * T / 2, (WIN_H - 1) * T);
                    /*
                    {
                        std::lock_guard<std::mutex> lock(player_mutex);
                        p.update_state(key_state, clk);
                    }
                    */
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

                if (!change_character) {
                    std::lock_guard<std::mutex> lock(player_mutex);
                    p.update_state(key_state, clk);
                }

                display_region(p, win, m5x7);

                // TODO check if either player reached target location (masala mix)
                // here. If one of them did, transition to GS_END
                if (winner >= 0) g_state = GS_END;
            } else if (g_state == GS_END) {
                // TODO check winner and print
                win.render_heading(
                    m5x7_l,
                    (winner == my_id ? "You Won!" : "Player " + std::to_string(winner) + " Won"),
                    WIN_W * T / 2, WIN_H * T / 2);
                win.render_text(m5x7, "Continue", WIN_W * T / 4, (WIN_H - 2) * T);
                win.render_text(m5x7, "Quit", 3 * WIN_W * T / 4, (WIN_H - 2) * T);
            } else if (g_state == GS_CAT) {
                SDL_SetRenderDrawColor(win.ren, 0x00, 0x00, 0x00, 120);
                SDL_Rect screen = {0, 0, WIN_W * T, WIN_H * T};
                SDL_RenderFillRect(win.ren, &screen);
                SDL_SetRenderDrawColor(win.ren, 0xFF, 0xFF, 0xFF, 0xFF);

                SDL_Rect frame = {T, 3 * T / 4, (WIN_W - 2) * T, WIN_H * T - (3 * T / 2)};
                SDL_RenderFillRect(win.ren, &frame);
                frame.x += 3 * T / 4;
                frame.y += 9 * T / 16;
                frame.w -= 3 * T / 2;
                frame.h -= 9 * T / 8;

                {
                    SDL_Surface* surf = TTF_RenderText_Solid(m5x7, "@iitd_cats", CL_BLACK);
                    SDL_Texture* texture = SDL_CreateTextureFromSurface(win.ren, surf);
                    SDL_Rect location_rect = {(WIN_W - 1) * T - surf->w, (WIN_H - 1) * T - T / 2,
                                              surf->w, surf->h};
                    SDL_RenderCopy(win.ren, texture, nullptr, &location_rect);
                    SDL_FreeSurface(surf);
                    SDL_DestroyTexture(texture);
                }

                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
                SDL_RenderCopy(win.ren, cat_img, nullptr, &frame);
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
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
                            if (g_state == GS_MMENU) {
                                change_character = false;
                                g_state = GS_FIND;
                            } else if (g_state == GS_END) {
                                // reset game
                                tgt_loc = -1;
                                winner = -1;
                                g_state = GS_WAIT;
                                camera = {13 * T, 78 * T, SCREEN_W, SCREEN_H};
                            }
                        } else if (inside(&BTN_RIGHT, xm / 4, ym / 4)) {
                            if (g_state == GS_MMENU) {
                                // create game
                                S.create();
                                connect_to_server("127.0.0.1");
                                change_character = false;
                                g_state = GS_WAIT;
                            } else if (g_state == GS_END) {
                                quit = true;
                                break;
                            }
                        }
                    }
                } else if (e.type == SDL_KEYUP) {
                    if (g_state == GS_MMENU || change_character) {
                        if (e.key.keysym.sym == SDLK_RIGHT) {
                            std::lock_guard<std::mutex> lock(player_mutex);
                            p.update_sprite(+1);
                        } else if (e.key.keysym.sym == SDLK_LEFT) {
                            std::lock_guard<std::mutex> lock(player_mutex);
                            p.update_sprite(-1);
                        }
                    }
                    if (change_character) {
                        if (e.key.keysym.sym == SDLK_RETURN) {
                            change_character = false;
                        }
                    }
                } else if (e.type == SDL_KEYDOWN) {
                    if (e.key.keysym.sym == SDLK_c) {
                        change_character = true;
                    }
                    if (e.key.keysym.sym == SDLK_r) {
                        // restart
                        g_state = GS_MMENU;
                        S.kill();
                        kill_client();

                        {
                            std::lock_guard<std::mutex> lock(others_mutex);
                            others.clear();
                        }

                        {
                            std::lock_guard<std::mutex> lock(player_mutex);
                            p.dir = DIR_D;
                        }

                        {
                            std::lock_guard<std::mutex> lock(cats_mutex);
                            cats.clear();
                        }
                    }
                    if (g_state == GS_FIND) {
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

                            connect_to_server(input_ip);
                            g_state = GS_WAIT;
                        }
                    } else if (g_state == GS_WAIT || g_state == GS_CHASE) {
                        if (e.key.keysym.sym == SDLK_RETURN) {
                            for (auto c : cats) {
                                SDL_Rect cat_rect = {c.pos_x * T - T / 2, c.pos_y * T - T / 2,
                                                     2 * T, 2 * T};
                                if (inside(&cat_rect, p.pos_x + T / 2, p.pos_y + T / 2)) {
                                    cat_img = cat_imgs[c.img];
                                    prev_state = g_state;
                                    g_state = GS_CAT;
                                    break;
                                }
                            }
                        }

                        if (g_state == GS_WAIT && e.key.keysym.sym == SDLK_s) {
                            // send request to server
                            // std::cout << "Sent request\n";
                            initiate_chase_request = true;
                        }
                    } else if (g_state == GS_CAT) {
                        g_state = prev_state;
                    }
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

        SDL_DestroyTexture(map);
        SDL_DestroyTexture(player_sprite);
        SDL_DestroyTexture(cat_sprite);
        for (auto& c : cat_imgs) SDL_DestroyTexture(c);

        TTF_CloseFont(m5x7);
        TTF_CloseFont(m5x7_l);
        TTF_CloseFont(minecraftia);

        kill_client();
    } catch (SDL_exception& e) {
        std::cerr << e.what() << std::endl;
    }

    (void)argc;
    (void)argv;
    return 0;
}
