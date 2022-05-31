SOURCES=$(wildcard src/*.cpp)
SOURCES+=$(wildcard vendor/*/*.cpp)
INCLUDES = $(shell pkg-config --libs sdl2)
CFLAGS = $(shell pkg-config --cflags sdl2)
CC = clang++

game:
	$(CC) -std=c++17 \
	-ggdb \
	-I/usr/local/Homebrew/include \
	$(INCLUDES) \
	$(CFLAGS) \
	-Iinclude \
	-Ivendor \
	$(SOURCES) \
	-o play -framework OpenGl