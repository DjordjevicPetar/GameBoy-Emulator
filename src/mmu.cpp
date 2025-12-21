#include "../inc/mmu.hpp"

MMU::MMU() {
    vram = vector<uint8_t>(VRAM_SIZE, 0);
    internal_ram = vector<uint8_t>(INTERNAL_RAM_SIZE, 0);
    sprite_attributes = vector<uint8_t>(SPRITE_ATTRIBUTES_SIZE, 0);
}

uint8_t MMU::read_memory_8(uint16_t addr) const {
    // ! Ovde mi puca program, kad pokrenem fetchOpcode()
    if (addr <= SWITCHABLE_ROM_END) {
        return cartridge.read8_rom(addr);
    }
    else if (addr <= VRAM_END) {
        return vram[addr - VRAM_START];
    }
    else if (addr <= SWITCHABLE_RAM_END) {
        return cartridge.read8_ram(addr);
    }
    else if (addr <= INTERNAL_RAM_END) {
        return internal_ram[addr - INTERNAL_RAM_START];
    }
    else if (addr <= SPRITE_ATTRIBUTES_END) {
        if (addr >= SPRITE_ATTRIBUTES_START) {
            return sprite_attributes[addr - SPRITE_ATTRIBUTES_START];
        }
    }
    else if (addr <= I_O_END) {
        if (addr >= I_O_START) {
            // TODO
            // TODO Timer
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            // TODO
        }
    }
    return 0xff; // placeholder
}

void MMU::write_memory_8(uint16_t addr, uint8_t val) {
    if (addr <= SWITCHABLE_ROM_END) {
        cartridge.write8_rom(addr, val);
    }
    // TODO Timer
}