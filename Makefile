# Detect OS
ifeq ($(OS),Windows_NT)
    TARGET := gameboy.exe
    RM := cmd /C del /Q
    RUN_PREFIX :=

    SDL_DIR := D:/Libraries/SDL3-3.2.26/i686-w64-mingw32
    SDL_INC := -I$(SDL_DIR)/include
    SDL_LIB := -L$(SDL_DIR)/lib -lSDL3
else
    TARGET := gameboy
    RM := rm -f
    RUN_PREFIX := ./

    # Hardcoded Homebrew paths for macOS (no need for pkg-config in PATH)
    HOMEBREW_PREFIX := /opt/homebrew
    SDL_INC := -I$(HOMEBREW_PREFIX)/include
    SDL_LIB := -L$(HOMEBREW_PREFIX)/lib -Wl,-rpath,$(HOMEBREW_PREFIX)/lib -lSDL3
endif

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

# Run with logging and compare to reference (headless for speed, 5 sec timeout)
diff: build
	-$(RM) cpu_log.txt
	@$(RUN_PREFIX)$(TARGET) $(ARGS) -l -h & PID=$$!; sleep 5; kill $$PID 2>/dev/null; wait $$PID 2>/dev/null || true
	@FIRST_DIFF=$$(diff cpu_log.txt EpicLog.txt 2>/dev/null | grep -m1 "^[0-9]" | cut -d',' -f1); \
	if [ -n "$$FIRST_DIFF" ]; then \
		echo "=== First difference at line: $$FIRST_DIFF ==="; \
		echo ""; \
		echo "--- cpu_log.txt (yours) ---"; \
		sed -n "$${FIRST_DIFF}p" cpu_log.txt; \
		echo ""; \
		echo "--- EpicLog.txt (reference) ---"; \
		sed -n "$${FIRST_DIFF}p" EpicLog.txt; \
	else \
		echo "No differences found or files missing"; \
	fi