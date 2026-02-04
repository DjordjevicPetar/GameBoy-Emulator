#pragma once

#include "../utils/types.hpp"


#include "../memory/memory_constants.hpp"
#include "../memory/cartridge.hpp"
#include "../ppu/ppu.hpp"
#include "../timer/timer.hpp"
#include "../joypad/joypad.hpp"
#include "../audio/apu.hpp"

class MMU {
public:
    MMU(std::string file_path, PPU* ppu, Timer* timer, InterruptController* interrupt_controller, Joypad* joypad, APU* apu);

    u8 read_memory_8(u16 addr) const;
    void write_memory_8(u16 addr, u8 val);
    
    // Serial output (for Blargg tests)
    std::string getSerialOutput() const { return serial_output_; }
    
private:
    Cartridge cartridge;
    InterruptController* interrupt_controller;
    APU* apu;
    Joypad* joypad;
    PPU* ppu;
    Timer* timer;

    std::vector<u8> wram;
    std::vector<u8> hram;
    
    // Serial port
    mutable u8 serial_data_ = 0;  // 0xFF01 - SB
    mutable u8 serial_control_ = 0;  // 0xFF02 - SC
    mutable std::string serial_output_;
};


