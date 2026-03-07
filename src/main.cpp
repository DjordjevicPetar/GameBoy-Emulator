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
