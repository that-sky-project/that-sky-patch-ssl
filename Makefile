# XY-Loader DLL — MinGW-w64 build (matches HTModLoader's toolchain).
CXX      ?= g++
CC       ?= gcc
OUT      := dist/ssl-patch.dll
SRC      := src/entry.cpp src/ssl/patch.cpp src/net/net.cpp src/config/config.cpp
CSRC     := vendor/cJSON/cJSON.c
INC      := -Ivendor/htmodloader -Ivendor -Isrc
LIBS     := vendor/htmodloader.lib
CXXFLAGS := -std=c++17 -O2 -m64 -Wall -Wextra $(INC)
CFLAGS   := -O2 -m64 $(INC)
LDFLAGS  := -shared -static -static-libgcc -static-libstdc++

all: $(OUT)

$(OUT): $(SRC) $(CSRC) $(wildcard src/**/*.hpp) $(wildcard src/*.hpp)
	@mkdir -p dist
	$(CXX) $(CXXFLAGS) $(SRC) $(CSRC) $(LIBS) $(LDFLAGS) -o $(OUT)

clean:
	rm -rf dist

.PHONY: all clean
