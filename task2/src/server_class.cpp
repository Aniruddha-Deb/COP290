#include "server_class.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "player.hpp"

server_class::server_class(bool start) {
    if (!start) return;

    std::future<void> future_obj = exit_signal.get_future();
    th = std::thread(&server_loop, std::move(future_obj));
}

std::optional<player> server_class::get_player(TCPsocket client) {
    char buf[packet_size + 1] = {};
    int len = SDLNet_TCP_Recv(client, buf, packet_size);

    if (len < (int)packet_size) return std::nullopt;
    return player::deserialize(buf);
}

std::optional<std::string> server_class::get_packet(TCPsocket client) {
    char buf[packet_size + 1] = {};
    int len = SDLNet_TCP_Recv(client, buf, packet_size);

    if (len < (int)packet_size) return std::nullopt;
    return std::string(buf);
}

std::string server_class::pad(std::string packet, size_t len) {
    assert(packet.size() <= len);
    packet.append(len - packet.size(), ' ');
    return packet;
}

void server_class::send_packet(TCPsocket client, std::string packet) {
    SDLNet_TCP_Send(client, pad(packet).c_str(), packet_size);
}

void server_class::server_loop(std::future<void> future_obj) {
    static constexpr auto loop_wait_time = std::chrono::milliseconds(1);

    IPaddress ip;
    SDLNet_ResolveHost(&ip, nullptr, server_class::port);
    TCPsocket server = SDLNet_TCP_Open(&ip);

    SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet(max_clients + 1);
    SDLNet_TCP_AddSocket(socket_set, server);

    std::vector<std::pair<TCPsocket, int>> clients;

    std::unordered_map<int, player> player_info;

    int last_id = 0;

    while (future_obj.wait_for(loop_wait_time) == std::future_status::timeout) {
        int num_ready = SDLNet_CheckSockets(socket_set, 5);
        if (num_ready <= 0) {
            continue;
        }

        if (SDLNet_SocketReady(server)) {
            auto client = SDLNet_TCP_Accept(server);
            if (client) {
                if (clients.size() == max_clients) {
                    SDLNet_TCP_Close(client);
                } else {
                    /*
                     * Receive initial state from them
                     */

                    SDLNet_TCP_AddSocket(socket_set, client);
                    num_ready = SDLNet_CheckSockets(socket_set, 100);
                    if (num_ready <= 0) {
                        SDLNet_TCP_Close(client);
                    } else {
                        auto p = get_player(client);
                        if (!p) {
                            SDLNet_TCP_Close(client);
                        } else {
                            if (p->id < 0) {
                                p->id = last_id + 1;
                                ++last_id;
                            }

                            player_info[p->id] = *p;
                            send_packet(client, p->serialize());

                            clients.emplace_back(client, p->id);
                            num_ready = SDLNet_CheckSockets(socket_set, 0);
                        }
                    }
                }
            }
        }

        for (size_t i = 0; i < clients.size(); ++i) {
            auto [client, id] = clients[i];
            if (SDLNet_SocketReady(client)) {
                auto p = get_player(client);
                if (p) {
                    // add it to the player database
                    player_info[id] = *p;
                } else {
                    // client is dead
                    player_info.erase(id);
                    clients.erase(clients.begin() + i);
                }
            }
        }

        // Now we must send all player's information to everyone
        for (auto [p_id, p] : player_info) {
            auto to_send = pad(p.serialize());
            for (auto [client, id] : clients) {
                if (id == p_id) continue;
                send_packet(client, to_send);
            }
        }
    }

    for (auto [client, _] : clients) SDLNet_TCP_Close(client);
    SDLNet_TCP_Close(server);
}

server_class::~server_class() {
    exit_signal.set_value();
    if (th.joinable()) th.join();
}
