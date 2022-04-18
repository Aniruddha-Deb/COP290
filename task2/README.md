# IITD Simulator

## Installation

This project requires SDL2, SDL2\_net, SDL2\_ttf and SDL2\_net to compile. 
Installation on mac: (with `brew`)
```
brew install sdl2
brew install sdl2_image
brew install sdl2_ttf
brew install sdl2_net
```
Installation on Linux:
```
sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-net-dev
```

After the libraries are installed, compile using the makefile: `make`. 

To run the game, use `start.sh`.

## Testing 

Tested on Ubuntu 20.04 and 21.10 (g++11), MacOS 10.15.6 Catalina (clang++). Working status
is in `status.txt`. 

## References

* [SDL2 API reference](https://wiki.libsdl.org/CategoryAPI)
* [LazyFoo tutorials on SDL](http://lazyfoo.net/SDL_tutorials/index.php)
* [SDL Tutorial 25 - networking with SDL\_net](https://www.youtube.com/watch?v=LNSqqxIKX_k&ab_channel=thecplusplusguy)
* [C++ Concurrency by Bo Qian](https://www.youtube.com/watch?v=LL8wkskDlbs&list=PL5jc9xFGsL8E12so1wlMS0r0hTQoJL74M&ab_channel=BoQian)
* [CPPReference.com](https://en.cppreference.com)
* Stack Overflow (duh)



