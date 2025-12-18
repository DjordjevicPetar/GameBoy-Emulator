#include "../inc/mmu.hpp"

uint8_t MMU::read8(uint16_t addr) {
    if (addr <= SWITCHABLE_ROM_END) {
        return cartridge.read8(addr);
    }
    return 0xff; // placeholder
}

void MMU::write8(uint16_t addr, uint8_t val) {
    if (addr <= SWITCHABLE_ROM_END) {
        cartridge.write8(addr, val);
    }
}