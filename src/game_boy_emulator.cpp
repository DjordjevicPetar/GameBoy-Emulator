#include "../inc/game_boy_emulator.hpp"
#include "../inc/cartridge.hpp"
#include <iostream>

GameBoyEmulator* GameBoyEmulator::instance = nullptr;

GameBoyEmulator::GameBoyEmulator() : cpu(&mmu) {
    this->stop_cpu = false;
    this->stop_gpu = false;
    this->cycles_executed = 0;
}

GameBoyEmulator* GameBoyEmulator::getInstance() {
    if (instance == nullptr) {
        instance = new GameBoyEmulator();
    }
    return instance;
}

void GameBoyEmulator::setFilepath(const std::string& filepath) {
    this->filepath = filepath;
}

void GameBoyEmulator::emulate() {
    Cartridge cartridge;
    cartridge.load_rom(filepath);
    
    // Main emulation loop
    while (true) {
        uint8_t cycles = cpu.execute_next_instruction();
        cycles_executed += cycles;
        
        // Handle timer
        timer.update_timer(cycles);
        
        // TODO: Handle GPU, Audio, etc.
        
        if (stop_cpu) {
            break;
        }
    }
}
