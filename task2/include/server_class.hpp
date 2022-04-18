#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <future>
#include <optional>
#include <string>
#include <thread>

#include "player.hpp"

struct server_class {
    static constexpr int port = 47854;
    static constexpr size_t max_clients = 4;
    static constexpr size_t packet_size = 128;

    std::promise<void> exit_signal;
    std::thread th;

    server_class();
    void create();
    void kill();
    static void server_loop(std::future<void> future_obj);
    ~server_class();

    static void send_packet(TCPsocket client, std::string);
    static std::optional<player> get_player(TCPsocket client);
    static std::optional<std::string> get_packet(TCPsocket client);

   private:
    static std::string pad(std::string, size_t = packet_size);
};
