TARGET := .\chip8.exe
SOURCES := $(wildcard src/*.cpp)
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall

clean: 
	if exist del $(TARGET)
build: clean
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)
run: build
	$(TARGET) $(ARGS)