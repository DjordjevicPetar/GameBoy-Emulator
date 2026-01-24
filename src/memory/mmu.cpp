#include "mmu.hpp"
#include <iostream>

MMU::MMU(std::string file_path, PPU* ppu, Timer* timer, InterruptController* interrupt_controller) :
    cartridge(file_path),
    interrupt_controller(interrupt_controller),
    ppu(ppu),
    timer(timer),
    wram(INTERNAL_RAM_SIZE, 0),
    hram(HIGH_RAM_SIZE, 0)
    {}

u8 MMU::read_memory_8(u16 addr) const {
    
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
    else if (addr <= ECHO_RAM_END) {
        return wram[addr - INTERNAL_RAM_SIZE - INTERNAL_RAM_START];
    }
    else if (addr <= OAM_END) {
        return ppu->read(addr);
    }
    else if (addr <= IO_END) {
        if (addr >= IO_START) {
            // Serial port
            if (addr == 0xFF01) {
                return serial_data_;
            }
            else if (addr == 0xFF02) {
                return serial_control_;
            }
            // Interrupt Flag register
            else if (addr == 0xFF0F) {
                return interrupt_controller->read_interrupt(addr);
            }
            else if (addr >= TIMER_REGS_START && addr <= TIMER_REGS_END) {
                return timer->read_timer(addr);
            }
            else if (addr >= PPU_REGS_START && addr <= PPU_REGS_END) {
                return ppu->read(addr);
            }
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            return hram[addr - HIGH_RAM_START];
        }
    }
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        return interrupt_controller->read_interrupt(addr);
    }
    return DEFAULT_READ_RETURN;
}

void MMU::write_memory_8(u16 addr, u8 val) {
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
    else if (addr <= ECHO_RAM_END) {
        wram[addr - INTERNAL_RAM_SIZE - INTERNAL_RAM_START] = val;
    }
    else if (addr <= OAM_END) {
        ppu->write(addr, val);
    }
    else if (addr <= IO_END) {
        if (addr >= IO_START) {
            // Serial port
            if (addr == 0xFF01) {
                serial_data_ = val;
            }
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
            // Interrupt Flag register
            else if (addr == 0xFF0F) {
                interrupt_controller->write_interrupt(addr, val);
            }
            else if (addr >= TIMER_REGS_START && addr <= TIMER_REGS_END) {
                timer->write_timer(addr, val);
            }
            else if (addr >= PPU_REGS_START && addr <= PPU_REGS_END) {
                return ppu->write(addr, val);
            }
        }
    }
    else if (addr <= HIGH_RAM_END) {
        if (addr >= HIGH_RAM_START) {
            hram[addr - HIGH_RAM_START] = val;
        }
    }
    else if (addr == INTERRUPT_REGISTER_ADDR) {
        interrupt_controller->write_interrupt(addr, val);
    }
}