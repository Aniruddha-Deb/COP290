#include "server_class.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <chrono>
#include <future>
#include <iostream>
#include <thread>

#include "player.hpp"

server_class::server_class(bool start) {
    if (!start) return;

    std::future<void> future_obj = exit_signal.get_future();
    th = std::thread(&server_loop, std::move(future_obj));
}

player server_class::get_player(TCPsocket client) {
    char buf[100];
    SDLNet_TCP_Recv(client, buf, 100);
    return player::deserialize(buf);
}

void server_class::server_loop(std::future<void> future_obj) {
    static constexpr auto loop_wait = std::chrono::milliseconds(16);

    IPaddress ip;
    SDLNet_ResolveHost(&ip, nullptr, 1234);
    TCPsocket server = SDLNet_TCP_Open(&ip);
    TCPsocket client;

    while (future_obj.wait_for(loop_wait) == std::future_status::timeout) {
        client = SDLNet_TCP_Accept(server);
        if (client) break;
    }
    if (!client) {
        SDLNet_TCP_Close(server);
        return;
    }

    while (future_obj.wait_for(loop_wait) == std::future_status::timeout) {
        auto cur = get_player(client);
        std::cout << cur.pos_x << ' ' << cur.pos_y << ' ' << cur.dir << ' ' << cur.iter << ' '
                  << cur.moving << '\n';
    }

    if (client) SDLNet_TCP_Close(client);
    SDLNet_TCP_Close(server);
}

server_class::~server_class() {
    exit_signal.set_value();
    th.join();
}
