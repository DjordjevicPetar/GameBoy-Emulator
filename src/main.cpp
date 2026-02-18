// TODO: Remove the #ifndef/#define/#endif include guard - this is a .cpp file,
// not a header. Include guards are only meaningful in headers.
#ifndef MAIN_H_
#define MAIN_H_

#include <iostream>
#include <thread>
#include <cstring>
#include "game_boy_emulator.hpp"
#include "utils/logger.hpp"

int main(int argc, char* argv[]){
    bool logging_enabled = false;
    const char* rom_path = nullptr;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-l") == 0) {
            logging_enabled = true;
        } else {
            rom_path = argv[i];
        }
    }

    // TODO: The -h (headless) flag is documented in the usage string but never parsed.
    // It's used in the Makefile's `diff` target but the emulator ignores it entirely.
    if (rom_path == nullptr) {
        std::cout << "ERROR: Program to execute not given" << std::endl;
        std::cout << "Usage: gameboy [-l] [-h] <rom_file>" << std::endl;
        std::cout << "  -l    Enable CPU logging to cpu_log.txt" << std::endl;
        std::cout << "  -h    Headless mode (no graphics, full speed)" << std::endl;
        return 1;
    }

    // Initialize logger
    Logger::init(logging_enabled);
    if (logging_enabled) {
        std::cout << "Logging enabled -> cpu_log.txt" << std::endl;
    }

    GameBoyEmulator::setFilepath(rom_path);
    GameBoyEmulator* emulator = GameBoyEmulator::getInstance();

    emulator->emulate();

    Logger::close();

    return 0;

}

#endif