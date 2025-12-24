#ifndef MAIN_H_
#define MAIN_H_

#include <iostream>
#include <thread>
#include <cstring>
#include "../inc/game_boy_emulator.hpp"
#include "../inc/logger.hpp"

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

    if (rom_path == nullptr) {
        std::cout << "ERROR: Program to execute not given" << std::endl;
        std::cout << "Usage: gameboy [-l] <rom_file>" << std::endl;
        std::cout << "  -l    Enable CPU logging to cpu_log.txt" << std::endl;
        return 1;
    }

    // Initialize logger
    Logger::init(logging_enabled);
    if (logging_enabled) {
        std::cout << "Logging enabled -> cpu_log.txt" << std::endl;
    }

    GameBoyEmulator* emulator = GameBoyEmulator::getInstance();

    emulator->setFilepath(rom_path);

    std::thread runningProgram(&GameBoyEmulator::emulate, emulator);

    runningProgram.join();

    Logger::close();

    return 0;

}

#endif