#ifndef MMU_HPP_
#define MMU_HPP_

#include "constants_mmu.hpp"
#include "cartridge.hpp"
#include "ppu.hpp"
#include "timer.hpp"
#include <cstdint>

class MMU {
public:
    MMU(std::string file_path, PPU* ppu, Timer* timer);

    uint8_t read_memory_8(uint16_t addr) const;
    void write_memory_8(uint16_t addr, uint8_t val);
private:
    Cartridge cartridge;
    PPU* ppu;
    Timer* timer;

    std::vector<uint8_t> wram;
    std::vector<uint8_t> hram;
};


#endif