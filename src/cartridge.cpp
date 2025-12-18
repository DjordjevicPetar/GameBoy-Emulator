#include "../inc/cartridge.hpp"

Cartridge::Cartridge() {
    currentBank = 1;
}

uint8_t Cartridge::read8(uint16_t addr) {
    if (addr <= STATIC_ROM_END) {
        return rom[addr];
    }
    else if (addr <= SWITCHABLE_ROM_END) {
        return rom[currentBank * SWITCHABLE_ROM_SIZE + (addr - SWITCHABLE_ROM_START)];
    }
    return 0xff; // might change
}

void Cartridge::write8(uint16_t addr, uint8_t val) {
    if (addr >= BANK_SWITCHING_START && addr <= BANK_SWITCHING_END) {
        currentBank = val & ROM_BANKS_MASK;
        if (currentBank == 0) currentBank = 1;
    }
}