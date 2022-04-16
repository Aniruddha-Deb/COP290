CXX = g++

NORMAL_FLAGS := -std=c++17 -O3
PEDANTIC_FLAGS := -Wall -Wextra -Werror -Wpedantic -pedantic-errors\
	-Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2\
	-Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC\
	-D_FORTIFY_SOURCE=2 -fsanitize=address -fsanitize=undefined -fno-sanitize-recover\
	-fstack-protector -g

ifeq ($(DEBUG), 1)
	CXXFLAGS := $(NORMAL_FLAGS) $(PEDANTIC_FLAGS)
else
	CXXFLAGS := $(NORMAL_FLAGS)
endif

SRC := src/server_class.cpp src/render_window.cpp src/player.cpp src/location_tagging.cpp src/main.cpp
OBJ := $(patsubst src/%.cpp, obj/%.o, $(SRC))

BIN := bin/game

LFLAGS := -lSDL2 -lSDL2_image -lSDL2_net -lSDL2_ttf -lm
LIB := -L/usr/lib/x86_64-linux-gnu/
INC := -I./include/ -I/usr/include/SDL2/

#This is the target that compiles our executable
all : $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $(INC) $(LIB) $^ -o $@ $(LFLAGS)

obj/%.o: src/%.cpp makefile
	$(CXX) $(CXXFLAGS) $(INC) $(LIB) -c $< -o $@ $(LFLAGS)

clean:
	$(RM) $(OBJ) $(BIN)
