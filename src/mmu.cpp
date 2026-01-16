#include "../inc/mmu.hpp"

MMU::MMU(std::string file_path, PPU* ppu, Timer* timer)
    : cartridge(file_path), // Interrupt Controller will probably be needed as well
      ppu(ppu),
      timer(timer),
      wram(INTERNAL_RAM_SIZE, 0),
      hram(HIGH_RAM_SIZE, 0)
    {}

uint8_t MMU::read_memory_8(uint16_t addr) const {
    
    if (addr <= SWITCHABLE_ROM_END) {
        return cartridge.read8(addr);
    }
    else if (addr <= VRAM_END) {
        return ppu->read(addr);
    }
    else if (addr <= SWITCHABLE_RAM_END) {
        return cartridge.read8(addr);
    }
    else if (addr <= INTERNAL_RAM_END) {
        return wram[addr - INTERNAL_RAM_START];
    }
    else if (addr <= OAM_END) {
        if (addr >= OAM_START) {
            return ppu->read(addr);
        }
    }
    else if (addr <= IO_END) {
        if (addr >= IO_START) {
            if (addr >= TIMER_REGS_START && addr <= TIMER_REGS_END) {
                timer->read_timer(addr);
            }
            else if (addr >= PPU_REGS_START && addr <= PPU_REGS_END) {
                return ppu->read(addr);
            }
            // TODO Other scopes (Interrupt Controller, etc)
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            return hram[addr - HIGH_RAM_START];
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
        ppu->write(addr, val);
    }
    else if (addr <= SWITCHABLE_RAM_END) {
        cartridge.write8(addr, val);
    }
    else if (addr <= INTERNAL_RAM_END) {
        wram[addr - INTERNAL_RAM_START] = val;
    }
    else if (addr <= OAM_END) {
        if (addr >= OAM_START) {
            ppu->write(addr, val);
        }
    }
    else if (addr <= IO_END) {
        if (addr >= IO_START) {
            if (addr >= TIMER_REGS_START && addr <= TIMER_REGS_END) {
                timer->write_timer(addr, val);
            }
            else if (addr >= PPU_REGS_START && addr <= PPU_REGS_END) {
                return ppu->write(addr, val);
            }
            // TODO Other scopes (Interrupt Controller, etc)
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            hram[addr - HIGH_RAM_START] = val;
        }
    }
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        // TODO
    }
}