#include "mmu.hpp"
#include <iostream>

MMU::MMU(std::string file_path, PPU* ppu, Timer* timer, InterruptController* interrupt_controller, Joypad* joypad, APU* apu) :
    cartridge(file_path),
    interrupt_controller(interrupt_controller),
    apu(apu),
    joypad(joypad),
    ppu(ppu),
    timer(timer),
    wram(INTERNAL_RAM_SIZE, 0),
    hram(HIGH_RAM_SIZE, 0)
    {}

// Dispatches reads to the appropriate subsystem bassed on address range.
u8 MMU::read_memory_8(u16 addr) const {
    
    // 0000-7FFF: ROM (fixed + switchable banks)
    if (addr <= SWITCHABLE_ROM_END) {
        return cartridge.read8(addr);
    }
    // 8000-9FFF: Video RAM
    else if (addr <= VRAM_END) {
        return ppu->read(addr);
    }
    // A000-BFFF: External (cartridge) RAM
    else if (addr <= SWITCHABLE_RAM_END) {
        return cartridge.read8(addr);
    }
    // C000-DFFF: Work RAM
    else if (addr <= INTERNAL_RAM_END) {
        return wram[addr - INTERNAL_RAM_START];
    }
    // E000-FDFF: Echo RAM (WRAM mirror)
    else if (addr <= ECHO_RAM_END) {
        return wram[addr - INTERNAL_RAM_SIZE - INTERNAL_RAM_START];
    }
    // FE00-FE9F: Object Attribute Memory (OAM)
    else if (addr <= OAM_END) {
        if (ppu->is_dma_active()) return DEFAULT_READ_RETURN;
        return ppu->read(addr);
    }
    // TODO: FEA0-FEFF is the "unusable" region. On DMG it returns 0x00 (not 0xFF).
    // Currently falls through to the I/O check below, which won't match, then to
    // HRAM check, which also won't match, returning 0xFF. Should explicitly handle
    // this range: if (addr >= 0xFEA0 && addr <= 0xFEFF) return 0x00;
    // FF00-FF7F: I/O registers
    else if (addr <= IO_END) {
        if (addr >= IO_START) {
            // Joypad Input
            if (addr == 0xFF00) {
                return joypad->read(addr);
            }
            // Serial Transfer Data
            else if (addr == 0xFF01) {
                return serial_data_;
            }
            // Serial Transfer Control
            else if (addr == 0xFF02) {
                return serial_control_;
            }
            // Timer registers
            else if (addr >= TIMER_REGS_START && addr <= TIMER_REGS_END) {
                return timer->read_timer(addr);
            }
            // Interrupt Flag (IF)
            else if (addr == 0xFF0F) {
                return interrupt_controller->read_interrupt(addr);
            }
            // Audio Processing Unit registers
            else if (addr >= APU_REGS_START && addr <= APU_REGS_END) {
                return apu->read(addr);
            }
            // LCD and Picture Processing Unit registers
            else if (addr >= PPU_REGS_START && addr <= PPU_REGS_END) {
                return ppu->read(addr);
            }
        }
    }
    // FF80-FFFE: High RAM (HRAM)
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            return hram[addr - HIGH_RAM_START];
        }
    }
    // FFFF: Interrupt Enable (IE)
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        return interrupt_controller->read_interrupt(addr);
    }
    
    // Unmapped or unused memory returns default value (0xFF)
    return DEFAULT_READ_RETURN;
}

// Dispatches writes to the appropriate subsystem bassed on address range.
void MMU::write_memory_8(u16 addr, u8 val) {
    // 0000-7FFF: ROM (bank control registers)
    if (addr <= SWITCHABLE_ROM_END) {
        cartridge.write8(addr, val);
    }
    // 8000-9FFF: Video RAM
    else if (addr <= VRAM_END) {
        ppu->write(addr, val);
    }
    // A000-BFFF: External (cartridge) RAM
    else if (addr <= SWITCHABLE_RAM_END) {
        cartridge.write8(addr, val);
    }
    // C000-DFFF: Work RAM
    else if (addr <= INTERNAL_RAM_END) {
        wram[addr - INTERNAL_RAM_START] = val;
    }
    // E000-FDFF: Echo RAM (WRAM mirror)
    else if (addr <= ECHO_RAM_END) {
        wram[addr - INTERNAL_RAM_SIZE - INTERNAL_RAM_START] = val;
    }
    // FE00-FE9F: Object Attribute Memory (OAM)
    else if (addr <= OAM_END) {
        if (ppu->is_dma_active()) return;
        ppu->write(addr, val);
    }
    // FF00-FF7F: I/O registers
    else if (addr <= IO_END) {
        if (addr >= IO_START) {
            // Joypad Input
            if (addr == 0xFF00) {
                joypad->write(addr, val);
            }
            else if (addr == 0xFF46) {
                // TODO(cycle-accuracy): After triggering DMA, reads from non-HRAM
                // should return 0xFF until the 160 M-cycle transfer completes.
                // See also PPU::write_dma() TODO.
                ppu->write_dma(val);
                return;
            }
            // Serial Transfer Data
            else if (addr == 0xFF01) {
                serial_data_ = val;
            }
            // Serial Transfer Control
            else if (addr == 0xFF02) {
                serial_control_ = val;
                // When bit 7 is set (0x80) and bit 0 is set (0x01), transfer starts
                if (val == 0x81) {
                    // Output the character
                    std::cout << static_cast<char>(serial_data_);
                    std::cout.flush();
                    serial_output_ += static_cast<char>(serial_data_);
                }
            }
            // Timer registers
            else if (addr >= TIMER_REGS_START && addr <= TIMER_REGS_END) {
                timer->write_timer(addr, val);
            }
            // Interrupt Flag (IF)
            else if (addr == 0xFF0F) {
                interrupt_controller->write_interrupt(addr, val);
            }
            // Audio Processing Unit registers
            else if (addr >= APU_REGS_START && addr <= APU_REGS_END) {
                apu->write(addr, val);
            }
            // LCD and Picture Processing Unit registers
            else if (addr >= PPU_REGS_START && addr <= PPU_REGS_END) {
                ppu->write(addr, val);
            }
        }
    }
    // FF80-FFFE: High RAM (HRAM)
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            hram[addr - HIGH_RAM_START] = val;
        }
    }
    // FFFF: Interrupt Enable (IE)
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        interrupt_controller->write_interrupt(addr, val);
    }
}