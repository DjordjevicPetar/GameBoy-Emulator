#ifndef MMU_HPP_
#define MMU_HPP_

#include "constants_mmu.hpp"
#include "cartridge.hpp"
#include <cstdint>

class MMU {
public:
    MMU(std::string file_path);

    uint8_t read_memory_8(uint16_t addr) const; // will separate based on address scope
    void write_memory_8(uint16_t addr, uint8_t val); // will separate based on address scope
private:
    Cartridge cartridge;
    std::vector<uint8_t> vram;
    std::vector<uint8_t> wram;
    std::vector<uint8_t> oam;
    std::vector<uint8_t> hram;
};


#endif