#include "server_class.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <cassert>
#include <chrono>
#include <future>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "location_tagging.hpp"
#include "player.hpp"

server_class::server_class() {}

void server_class::create() {
    std::future<void> future_obj = exit_signal.get_future();
    th = std::thread(&server_loop, std::move(future_obj));
}

std::optional<player> server_class::get_player(TCPsocket client) {
    char buf[packet_size + 1] = {};
    int len = SDLNet_TCP_Recv(client, buf, packet_size);

    if (len < (int)packet_size || buf[0] != 'P') return std::nullopt;
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

    std::vector<int> dead_clients;
    int tgt_loc = -1;

    while (future_obj.wait_for(loop_wait_time) == std::future_status::timeout) {
        int num_ready = SDLNet_CheckSockets(socket_set, 0);
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
                    num_ready = SDLNet_CheckSockets(socket_set, 0);
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

        bool initiate_chase = false;
        for (size_t i = 0; i < clients.size(); ++i) {
            auto [client, id] = clients[i];
            if (SDLNet_SocketReady(client)) {
                auto new_packet = get_packet(client);
                if (new_packet) {
                    char opt = (*new_packet)[0];
                    if (opt == 'P') {
                        // add it to the player database
                        player_info[id] = player::deserialize(*new_packet);
                    } else if (opt == 'I') {
                        // tell everyone that a game is to be started
                        initiate_chase = true;
                    } else {
                        assert(false);
                    }
                } else {
                    // client is dead

                    // tell everyone that client is dead
                    dead_clients.push_back(id);

                    // erase info about client
                    player_info.erase(id);
                    clients.erase(clients.begin() + i);
                }
            }
        }

        // Now we must send all player's information to everyone
        for (auto [p_id, p] : player_info) {
            const auto to_send = pad(p.serialize());
            for (const auto &[client, id] : clients) {
                if (id == p_id) continue;
                send_packet(client, to_send);
            }
        }

        // tell about dead clients
        for (auto dead_id : dead_clients) {
            auto to_send = pad("D " + std::to_string(dead_id));
            for (auto [client, id] : clients) {
                send_packet(client, to_send);
            }
        }
        dead_clients.clear();

        if (initiate_chase) {
            if (clients.size() == 2) {
                // can't start chase if != 2 players

                tgt_loc = get_target_location();
                const auto spawns = get_spawn_points_for_location(tgt_loc);

                std::stringstream ss;

                ss << "I " << tgt_loc;

                int idx = 0;
                for (const auto &[client, id] : clients) {
                    ss << ' ' << id << ' ' << spawns[idx].first << ' ' << spawns[idx].second;
                    ++idx;
                }
                assert(idx == 2);

                const auto to_send = ss.str();
                for (const auto &[client, id] : clients) send_packet(client, to_send);

                // std::cout << "Sent chase packet: " << to_send << '\n';
            } else {
                // std::cout << "Ignored request\n";
            }
            initiate_chase = false;
        }

        if (tgt_loc >= 0) {
            for (const auto &[p_id, p] : player_info) {
                auto loc = get_region(p);
                if (loc == tgt_loc) {
                    // tell everyone that p_id wins
                    const auto to_send = "W " + std::to_string(p_id);
                    for (const auto &[client, id] : clients) send_packet(client, to_send);
                    // race is over
                    tgt_loc = -1;
                    break;
                }
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
