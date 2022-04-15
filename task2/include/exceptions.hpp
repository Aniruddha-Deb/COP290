#pragma once

#include <exception>
#include <string>

struct SDL_exception : public std::exception {
    const std::string _what;
    SDL_exception(const std::string& msg)
        : _what{msg + "\nSDL_GetError(): " + std::string(SDL_GetError())} {}
    const char* what() const throw() { return _what.c_str(); }
};
