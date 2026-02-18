#pragma once

#include "utils/types.hpp"


#include "cpu/cpu.hpp"
#include "memory/mmu.hpp"
#include "timer/timer.hpp"
#include "interrupt/interrupt_controller.hpp"
#include <string>
#include <SDL3/SDL.h>
#include <deque>

class GameBoyEmulator {
public:
    GameBoyEmulator();
    
    // Disable copy and move
    GameBoyEmulator(const GameBoyEmulator&) = delete;
    GameBoyEmulator& operator=(const GameBoyEmulator&) = delete;
    GameBoyEmulator(GameBoyEmulator&&) = delete;
    GameBoyEmulator& operator=(GameBoyEmulator&&) = delete;
    
    void emulate();
    void run_until_next_frame();  // Public for headless mode

    static void setFilepath(const std::string& filepath);
    
    // TODO: Singleton with raw `new` leaks memory (instance_ is never deleted).
    // Either use a static local variable or std::unique_ptr for the instance.
    static GameBoyEmulator* getInstance();

private:
    // Components (order matters for initialization!)
    InterruptController interrupt_controller_;
    Joypad joypad_;
    APU apu_;
    PPU ppu_;
    Timer timer_;
    MMU mmu_;
    CPU cpu_;
    
    void check_events();

    // State
    bool stop_cpu_ = false;
    u32 cycles_executed_ = 0;
    static std::string filepath_;

    // Audio
    u32 cycles_from_audio_sample = 0;
    std::vector<s16> audio_buffer;
    
    static GameBoyEmulator* instance_;
};

