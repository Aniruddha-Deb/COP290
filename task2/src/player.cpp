#include "player.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <algorithm>
#include <cassert>
#include <sstream>

#include "globals.hpp"
#include "render_window.hpp"

void player::update_sprite(int incr) { character = (character + 3 + incr) % 3; }

void player::update_state(const Uint8* state, int clk, gameStates g) {
    int px = (pos_x + T) / T;
    int py = (pos_y + T) / T;
    int xe = pos_x % T;
    int ye = pos_y % T;

    int speed = (g == GS_CHASE) ? 1 : (state[SDL_SCANCODE_SPACE] ? 2 : 1);

    if (state[SDL_SCANCODE_RIGHT] || state[SDL_SCANCODE_LEFT] || state[SDL_SCANCODE_UP] ||
        state[SDL_SCANCODE_DOWN]) {
        moving = true;
        if (state[SDL_SCANCODE_RIGHT])
            dir = DIR_R;
        else if (state[SDL_SCANCODE_LEFT])
            dir = DIR_L;
        else if (state[SDL_SCANCODE_UP])
            dir = DIR_U;
        else if (state[SDL_SCANCODE_DOWN])
            dir = DIR_D;
    } else {
        moving = false;
    }

    if (state[SDL_SCANCODE_RIGHT]) {
        if (walkable[py][px + 1] && walkable[py + 1][px + 1]) {
            pos_x += speed;
        } else if (walkable[py][px + 1] && !walkable[py + 1][px + 1] && ye < 4) {
            pos_x += speed;
            pos_y = (py - 1) * T;
        } else if (walkable[py + 1][px + 1] && !walkable[py][px + 1] && ye >= 12) {
            pos_x += speed;
            pos_y = py * T;
        }
    } else if (state[SDL_SCANCODE_LEFT]) {
        if (walkable[py][px] && walkable[py + 1][px]) {
            pos_x -= speed;
        } else if (walkable[py][px] && !walkable[py + 1][px] && ye < 4) {
            pos_x -= speed;
            pos_y = (py - 1) * T;
        } else if (walkable[py + 1][px] && !walkable[py][px] && ye >= 12) {
            pos_x -= speed;
            pos_y = py * T;
        }
    } else if (state[SDL_SCANCODE_UP]) {
        if (walkable[py][px] && walkable[py][px + 1]) {
            pos_y -= speed;
        } else if (walkable[py][px] && !walkable[py][px + 1] && xe < 4) {
            pos_y -= speed;
            pos_x = (px - 1) * T;
        } else if (walkable[py][px + 1] && !walkable[py][px] && xe >= 12) {
            pos_y -= speed;
            pos_x = px * T;
        }
    } else if (state[SDL_SCANCODE_DOWN]) {
        if (walkable[py + 1][px] && walkable[py + 1][px + 1]) {
            pos_y += speed;
        } else if (walkable[py + 1][px] && !walkable[py + 1][px + 1] && xe < 4) {
            pos_y += speed;
            pos_x = (px - 1) * T;
        } else if (walkable[py + 1][px + 1] && !walkable[py + 1][px] && xe >= 12) {
            pos_y += speed;
            pos_x = px * T;
        }
    }

    if (clk % 4 == 0) {
        if (moving)
            iter = (iter + 1) % 4;
        else
            iter = 0;
    }
}

SDL_Rect player::get_camera() {
    // ideally, you want src to be centered at (CW, CH)
    SDL_Rect camera;
    camera.x = pos_x - SCREEN_W / 2;
    camera.x = std::clamp(camera.x, 0, (MAP_W - WIN_W) * T);

    camera.y = pos_y - SCREEN_H / 2;
    camera.y = std::clamp(camera.y, 0, (MAP_H - WIN_H) * T);

    camera.w = SCREEN_W;
    camera.h = SCREEN_H;
    return camera;
}

void player::render(render_window& win, const SDL_Rect& camera, SDL_Texture* player_sprite) {
    SDL_Rect dst = {pos_x - camera.x, pos_y - camera.y, W, H};

    SDL_Rect src = {(dir + 4 * conv[iter]) * W, character * H, W, H};
    SDL_RenderCopy(win.ren, player_sprite, &src, &dst);
}

std::string player::serialize() {
    std::stringstream ss;
    ss << "P " << pos_x << ' ' << pos_y << ' ' << dir << ' ' << (moving ? 1 : 0) << ' ' << iter
       << ' ' << id << ' ' << character;
    return ss.str();
}

player player::deserialize(std::string s) {
    std::stringstream ss;
    ss << s;

    char c;
    ss >> c;
    assert(c == 'P');

    player p;

    ss >> p.pos_x >> p.pos_y;

    int t;

    ss >> t;
    p.dir = (directions)t;

    ss >> t;
    p.moving = t;

    ss >> p.iter >> p.id >> p.character;

    return p;
}
