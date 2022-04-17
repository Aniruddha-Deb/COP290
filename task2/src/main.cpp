#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <chrono>
#include <iostream>
#include <random>
#include <algorithm> 
#include <array>

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

bool inside(const SDL_Rect* r, int x, int y) {
    return x > r->x && x < r->x + r->w && y > r->y && y < r->y + r->h;
}

void spawn_cats(int (&catlocs)[4][2], int* catimgs, int ncats) {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> locs(0,116*32-1);
    std::array<int,10> cats {1,2,3,4,5,6,7,8,9,10};
    shuffle (cats.begin(), cats.end(), rng);
    for (int i=0; i<ncats;) {
        int loc = locs(rng);
        if (walkable[loc/MAP_W + 1][loc%MAP_W + 1]) {
            // location found for cat
            i++;
            catlocs[i][0] = loc%MAP_W;
            catlocs[i][1] = loc/MAP_W;
            catimgs[i] = cats[i];
        }
    }
}

int main(int argc, char* argv[]) {
    try {
        bool start_server = ask_server();
        gameStates g_state = GS_MMENU;

        render_window win("IITD Simulator", T * WIN_W * SCALE, T * WIN_H * SCALE);
        win.scale(SCALE);

        server_class S(start_server);  // start the server if start_server is true

        const Uint8* key_state = SDL_GetKeyboardState(nullptr);

        auto map = win.load_texture("../assets/iitd_map.png");
        SDL_Rect camera = {13 * T, 78 * T, SCREEN_W, SCREEN_H};

        auto player_sprite = win.load_texture("../assets/characters.png");
        player p(camera.x + T * WIN_W / 2,
                 camera.y + T * WIN_H / 2);  // change this to change start location

        auto m5x7 = win.load_font("../assets/m5x7.ttf", 16);
        auto m5x7_l = win.load_font("../assets/m5x7.ttf", 32);
        auto minecraftia = win.load_font("../assets/Minecraftia-Regular.ttf", 16);

        SDL_Event e;
        bool quit = false;

        int clk = 0;

        int ncats = 4;
        int catlocs[4][2];
        int catimgs[4];
        spawn_cats(catlocs, catimgs, ncats);
        auto cat_texture = win.load_texture("../assets/cats.png");
        int curr_cat = -1;
        SDL_Texture* cat_img;
        gameStates prev_state = GS_MMENU;

        auto start_time = std::chrono::steady_clock::now().time_since_epoch();

        IPaddress server_ip;
        SDLNet_ResolveHost(&server_ip, "127.0.0.1", 1234);
        TCPsocket server = SDLNet_TCP_Open(&server_ip);

        int cntdwn_timer = 3;
        int tgt_loc;
        std::pair<std::pair<int,int>,std::pair<int,int>> spawns;

        std::string ip = "";

        while (!quit) {
            clk = (clk + 1) % 4;

            camera = p.get_camera();
            p.send_player(server);

            win.clear();
            SDL_RenderCopy(win.ren, map, &camera, nullptr);
            p.render(win, clk, camera, player_sprite);
            for (int i=0; i<ncats; i++) {
                SDL_Rect srcrect = {(i%4)*T,0,T,T};
                SDL_Rect dstrect = {(catlocs[i][0]*T-camera.x),(catlocs[i][1]*T-camera.y),T,T};
                if (inside(&camera, catlocs[i][0]*T, catlocs[i][1]*T) || inside(&camera, catlocs[i][0]*T+T, catlocs[i][1]*T+T)) {
                    SDL_RenderCopy(win.ren, cat_texture, &srcrect, &dstrect);
                }
            }

            if (g_state == GS_MMENU) {
                win.render_heading(m5x7_l, "IITD Simulator", WIN_W*T/2, T);
                win.render_text(m5x7, "Find a Game", WIN_W*T/4, (WIN_H-2)*T);
                win.render_text(m5x7, "Create a Game", 3*WIN_W*T/4, (WIN_H-2)*T);
                // m5x7 doesn't have < and > glyphs, sad
                win.render_text(minecraftia, ">", (WIN_W/2+1)*T + T/2, WIN_H*T/2 + T/2);
                win.render_text(minecraftia, "<", (WIN_W/2-1)*T + T/2, WIN_H*T/2 + T/2);
            }
            else if (g_state == GS_WAIT) {
                win.render_text(m5x7, "IP: 127.0.0.1", 3*T,T/2);
                win.render_text(m5x7, "1/2", (WIN_W-1)*T,T/2);
                p.update_state(key_state); // free roam mode (without being able to see regions) 
                                           // for the server creator
                // TODO check if the number of people connected is 2/2
                // If so, enter countdown mode in both clients (send a message across)
            }
            else if (g_state == GS_CAT) {
                SDL_RenderCopy(win.ren,cat_img,NULL,NULL);
            }
            else if (g_state == GS_COUNTDOWN) {
                if (cntdwn_timer == 0) {
                    // TODO server should send a packet informing both players of the
                    // target location to reach, and a spawn point for each of 
                    // them (chosen equidistantly?)
                    tgt_loc = get_target_location();
                    spawns = get_spawn_points_for_location(tgt_loc);
                    std::cout << "(" << spawns.first.first << "," << spawns.first.second << "),(" << spawns.second.first << "," << spawns.second.second << ")" << std::endl;
                    g_state = GS_CHASE;
                }
                else {
                    win.render_text(m5x7, "Starting in " + std::to_string(cntdwn_timer), WIN_W*T/2, (WIN_H-1)*T );
                    p.update_state(key_state);
                }
            }
            else if (g_state == GS_FIND) {
                win.render_text(m5x7, "Enter IP address", WIN_W*T/2, WIN_H*T/4);
                SDL_RenderDrawLine(win.ren, (WIN_W-4)*T/2, WIN_H*T/4 + 3*T/2, (WIN_W+4)*T/2, WIN_H*T/4 + 3*T/2);
                if (ip.length() > 0) {
                    win.render_text(m5x7, ip.c_str(), WIN_W*T/2, WIN_H*T/4 + T);
                }
            }
            else if (g_state == GS_CHASE) {
                std::string prompt = str_regions[tgt_loc];
                prompt = "Get to " + prompt;
                win.render_text(m5x7, prompt.c_str(), WIN_W*T/2, T);
                p.update_state(key_state);
                display_region(p, win, m5x7);
                // TODO check if either player reached target location (masala mix)
                // here. If one of them did, transition to GS_END
            }
            else if (g_state == GS_END) {
                // TODO check winner and print
                win.render_heading(m5x7_l, "Player 1 Won", WIN_W*T/2, WIN_H*T/2);
                win.render_text(m5x7, "Main Menu", WIN_W*T/4, (WIN_H-2)*T);
                win.render_text(m5x7, "Quit", 3*WIN_W*T/4, (WIN_H-2)*T);                
            }

            win.display();

            while (SDL_PollEvent(&e) != 0) {
                if (e.type == SDL_QUIT) {
                    quit = true;
                    break;
                }
                else if (e.type == SDL_MOUSEBUTTONDOWN) {
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        int xm, ym;
                        SDL_GetMouseState(&xm,&ym);
                        if (inside(&BTN_LEFT, xm/4, ym/4)) {
                            if (g_state == GS_MMENU) g_state = GS_FIND;
                            else if (g_state == GS_END) {
                                // reset game
                                g_state = GS_MMENU;
                                cntdwn_timer = 3;
                                camera = {13 * T, 78 * T, SCREEN_W, SCREEN_H};
                            }
                        }
                        else if (inside(&BTN_RIGHT, xm/4, ym/4)) {
                            if (g_state == GS_MMENU) {
                                g_state = GS_WAIT;
                            }
                            else if (g_state == GS_END) {
                                quit = true;
                                break;
                            }
                        }
                    }
                }
                else if (e.type == SDL_KEYUP) {
                    if (g_state == GS_MMENU) {
                        if (e.key.keysym.sym == SDLK_RIGHT) p.update_sprite(1);
                        else if (e.key.keysym.sym == SDLK_LEFT) p.update_sprite(-1);
                    }
                }
                else if (e.type == SDL_KEYDOWN) {
                    if (g_state == GS_FIND) {
                        if ((e.key.keysym.sym >= SDLK_0 && e.key.keysym.sym <= SDLK_9) || e.key.keysym.sym == SDLK_PERIOD) {
                            if (ip.length() < 15) {
                                ip.push_back(e.key.keysym.sym);
                            }
                        }
                        if (e.key.keysym.sym == SDLK_BACKSPACE) {
                            if (ip.length() > 0) ip.erase(ip.length()-1);
                        }
                        if (e.key.keysym.sym == SDLK_RETURN) {
                            // TODO connect to other server here and await countdown
                            // mode confirmation
                            g_state = GS_COUNTDOWN;
                        }
                    }
                    else if (g_state == GS_WAIT || g_state == GS_CHASE) {
                        // cat check
                        for (int i=0; i<ncats; i++) {
                            SDL_Rect cat_rect = {catlocs[i][0]*T-T/2,catlocs[i][1]*T-T/2,2*T,2*T};
                            if (inside(&cat_rect,p.pos_x+8,p.pos_y+8) && e.key.keysym.sym == SDLK_SPACE) {
                                // cat collision
                                prev_state = g_state;
                                g_state = GS_CAT;
                                curr_cat = i;
                                // render the current cat onto screen
                                std::string catloc = "../assets/cat" + std::to_string(catimgs[i]) + ".png";
                                cat_img = win.load_texture(catloc);
                                break;
                            }
                        }
                    }
                    else if (g_state == GS_CAT) {
                        g_state = prev_state;
                    }
                }
            }

            // TODO please find a better way of doing this XD. Have to time it for
            // 3 seconds.
            if (clk == 0 && g_state == GS_COUNTDOWN) cntdwn_timer--; 

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

        SDLNet_TCP_Close(server);

    } catch (SDL_exception& e) {
        std::cerr << e.what() << std::endl;
    }

    (void)argc;
    (void)argv;
    return 0;
}
