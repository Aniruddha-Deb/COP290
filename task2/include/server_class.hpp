#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include <future>
#include <thread>

#include "player.hpp"

struct server_class {
    static constexpr int port = 1234;

    std::promise<void> exit_signal;
    std::thread th;

    server_class(bool start);
    static player get_player(TCPsocket client);
    static void server_loop(std::future<void> future_obj);
    ~server_class();
};
