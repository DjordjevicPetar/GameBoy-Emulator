#include "../inc/mmu.hpp"

MMU::MMU(std::string file_path)
    : cartridge(file_path),
      vram(VRAM_SIZE, 0),
      wram(INTERNAL_RAM_SIZE, 0),
      oam(SPRITE_ATTRIBUTES_SIZE, 0),
      hram(HIGH_RAM_SIZE, 0)
    {}

uint8_t MMU::read_memory_8(uint16_t addr) const {
    
    if (addr <= SWITCHABLE_ROM_END) {
        return cartridge.read8(addr);
    }
    else if (addr <= VRAM_END) {
        return vram[addr - VRAM_START]; // TODO
    }
    else if (addr <= SWITCHABLE_RAM_END) {
        return cartridge.read8(addr);
    }
    else if (addr <= INTERNAL_RAM_END) {
        return wram[addr - INTERNAL_RAM_START];
    }
    else if (addr <= SPRITE_ATTRIBUTES_END) {
        if (addr >= SPRITE_ATTRIBUTES_START) {
            return oam[addr - SPRITE_ATTRIBUTES_START]; // TODO
        }
    }
    else if (addr <= I_O_END) {
        if (addr >= I_O_START) {
            // TODO Timer, Interrupt Controller
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            return hram[addr - HIGH_RAM_START]; // TODO
        }
    }
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        // TODO
    }
    return DEFAULT_READ_RETURN;
}

void MMU::write_memory_8(uint16_t addr, uint8_t val) {
    if (addr <= SWITCHABLE_ROM_END) {
        cartridge.write8(addr, val);
    }
    else if (addr <= VRAM_END) {
        vram[addr - VRAM_START] = val; // TODO
    }
    else if (addr <= SWITCHABLE_RAM_END) {
        cartridge.write8(addr, val);
    }
    else if (addr <= INTERNAL_RAM_END) {
        wram[addr - INTERNAL_RAM_START] = val;
    }
    else if (addr <= SPRITE_ATTRIBUTES_END) {
        if (addr >= SPRITE_ATTRIBUTES_START) {
            oam[addr - SPRITE_ATTRIBUTES_START] = val; // TODO
        }
    }
    else if (addr <= I_O_END) {
        if (addr >= I_O_START) {
            // TODO Timer, Interrupt Controller
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            hram[addr - HIGH_RAM_START] = val; // TODO
        }
    }
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        // TODO
    }
}