#ifndef EMULATOR_H_
#define EMULATOR_H_

#include "../inc/cpu.hpp"
#include "../inc/mmu.hpp"
#include "../inc/timer.hpp"
#include "../inc/interrupt_controller.hpp"
#include <string>

class GameBoyEmulator {
private:
    CPU cpu;
    MMU mmu;
    Timer timer;
    InterruptController interrupt_controller;
    // Stop conditions
    bool stop_cpu;
    bool stop_gpu;
    
    uint32_t cycles_executed;
    std::string filepath;
    
    static GameBoyEmulator* instance;
    
public:
    GameBoyEmulator();
    GameBoyEmulator(GameBoyEmulator&) = delete;
    GameBoyEmulator(GameBoyEmulator&&) = delete;
    
    void emulate();
    void setFilepath(const std::string& filepath);
    static GameBoyEmulator* getInstance();
};

#endif
