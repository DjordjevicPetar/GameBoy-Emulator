#ifndef MMU_HPP_
#define MMU_HPP_

#include "constants_mmu.hpp"
#include "cartridge.hpp"
#include <cstdint>

class MMU {
public:
    uint8_t read8(uint16_t addr); // will separate based on address scope
    void write8(uint16_t addr, uint8_t val); // will separate based on address scope
private:
    Cartridge cartridge;
};


#endif