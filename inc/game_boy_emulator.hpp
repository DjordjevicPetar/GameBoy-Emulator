#ifndef GAME_BOY_EMULATOR_HPP_
#define GAME_BOY_EMULATOR_HPP_

#include "cpu.hpp"
#include "mmu.hpp"
#include "timer.hpp"
#include "interrupt_controller.hpp"
#include <cstdint>
#include <string>
#include <SDL3/SDL.h>

class GameBoyEmulator {
public:
    GameBoyEmulator();
    
    // Disable copy and move
    GameBoyEmulator(const GameBoyEmulator&) = delete;
    GameBoyEmulator& operator=(const GameBoyEmulator&) = delete;
    GameBoyEmulator(GameBoyEmulator&&) = delete;
    GameBoyEmulator& operator=(GameBoyEmulator&&) = delete;
    
    void emulate();
    

    static void setFilepath(const std::string& filepath);
    
    static GameBoyEmulator* getInstance();

private:
    // Components (order matters for initialization!)
    InterruptController interrupt_controller_;
    PPU ppu_;
    Timer timer_;
    MMU mmu_;
    CPU cpu_;
    
    bool run_until_next_frame();
    bool check_quit_request();

    // State
    bool stop_cpu_ = false;
    bool stop_gpu_ = false;
    uint32_t cycles_executed_ = 0;
    static std::string filepath_;
    
    static GameBoyEmulator* instance_;
};

#endif
