# Windows
ifeq ($(OS),Windows_NT)
    TARGET := gameboy.exe
    RM := cmd /C del /Q
    RUN_PREFIX :=

    SDL_DIR := D:/Libraries/SDL3-3.2.26/i686-w64-mingw32
    SDL_INC := -I$(SDL_DIR)/include
    SDL_LIB := -L$(SDL_DIR)/lib -lSDL3

SOURCES := $(shell bash -c "find src -name '*.cpp'")
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -I./inc $(SDL_INC)

.PHONY: clean build run

clean:
	-$(RM) $(TARGET)

build: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SDL_LIB)

run: build
	$(RUN_PREFIX)$(TARGET) $(ARGS) -l

# MacOS/Linux	
else
    TARGET := gameboy
    RM := rm -f
    RUN_PREFIX := ./

    SDL_INC := $(shell pkg-config sdl3 --cflags)
    SDL_LIB := $(shell pkg-config sdl3 --libs)

# Find all .cpp files in src/ and subdirectories
SOURCES := $(shell find src -name '*.cpp')
CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -I./src $(SDL_INC)

.PHONY: clean build run diff

clean:
	-$(RM) $(TARGET) cpu_log.txt

build: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET) $(SDL_LIB)

run: build
	$(RUN_PREFIX)$(TARGET) $(ARGS)
endif

