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
    vector<uint8_t> vram;
    vector<uint8_t> internal_ram;
    vector<uint8_t> sprite_attributes;
};


#endif