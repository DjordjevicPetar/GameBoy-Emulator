#include "../inc/game_boy_emulator.hpp"
#include "../inc/cartridge.hpp"
#include <iostream>

GameBoyEmulator* GameBoyEmulator::instance_ = nullptr;

GameBoyEmulator::GameBoyEmulator() 
    : interrupt_controller_()
    , mmu_()
    , cpu_(&mmu_, &interrupt_controller_)
    , timer_(&interrupt_controller_) {}

GameBoyEmulator* GameBoyEmulator::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new GameBoyEmulator();
    }
    return instance_;
}

void GameBoyEmulator::setFilepath(const std::string& filepath) {
    filepath_ = filepath;
}

void GameBoyEmulator::emulate() {
    Cartridge cartridge;
    cartridge.load_rom(filepath_);
    
    // Main emulation loop
    while (!stop_cpu_) {
        uint8_t cycles = cpu_.execute_next_instruction();
        cycles += cpu_.handle_interrupts();
        cycles_executed_ += cycles;
        
        // Handle timer
        timer_.update_timer(cycles);
        
        // TODO: Handle GPU, Audio, etc.
    }
}
