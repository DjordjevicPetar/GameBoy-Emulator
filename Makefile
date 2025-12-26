# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET := gameboy.exe
    RM := cmd /C del /Q
    RUN_PREFIX :=
else
    TARGET := gameboy
    RM := rm -f
    RUN_PREFIX := ./
endif

SOURCES := $(filter-out src/mmu_main.cpp,$(wildcard src/*.cpp))
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -I./inc

.PHONY: clean build run

clean:
	-$(RM) $(TARGET)

build: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

run: build
	$(RUN_PREFIX)$(TARGET) $(ARGS) -l