#include "../inc/mbc.hpp"

MBC0::MBC0(vector<uint8_t>& rom, vector<uint8_t>& ram) : rom(rom), ram(ram) {}

uint8_t MBC0::read(uint16_t addr) { // 2 ROM banks
    if (addr <= SWITCHABLE_ROM_END) {
        return rom[addr];
    }
    else if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END && !ram.empty()) {
        return ram[addr - SWITCHABLE_RAM_START];
    }
    return DEFAULT_READ_RETURN;
}

void MBC0::write(uint16_t addr, uint8_t val) {
    if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END) {
        ram[addr - SWITCHABLE_RAM_START] = val;
    }
}

MBC1::MBC1(vector<uint8_t>& rom, vector<uint8_t>& ram) : rom(rom), ram(ram) {
    rom_banks = rom.size() / SWITCHABLE_ROM_SIZE;
}

uint8_t MBC1::read(uint16_t addr) {
    if (addr <= STATIC_ROM_END) {
        return rom[addr];
    }
    else if (addr <= SWITCHABLE_ROM_END) {
        return rom[current_rom_bank_low * SWITCHABLE_ROM_SIZE + (addr - SWITCHABLE_ROM_START)];
    }
    else if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END) {
        return ram[current_ram_bank * SWITCHABLE_RAM_SIZE + (addr - SWITCHABLE_RAM_START)];
    }
    return DEFAULT_READ_RETURN; 
}

void MBC1::write(uint16_t addr, uint8_t val) {
    if (addr <= RAM_ENABLE_END) {
        ram_enabled = (val & MBC1_RAM_ENABLE_MASK) == MBC1_RAM_ENABLE_ENABLED;
    }
    else if (addr <= ROM_BANK_SELECT_END) {
        current_rom_bank_low = val & MBC1_ROM_BANKS_MASK;
        if (current_rom_bank_low == 0) current_rom_bank_low = 1;
        current_rom_bank_low %= rom_banks;
    }
    else if (addr <= RAM_BANK_SELECT_END) {
        if (!banking_mode) {
            current_rom_bank_high = val & 0x03;
        }
        else {
            current_ram_bank = val & MBC1_RAM_BANKS_MASK;
        }
    }
    else if (addr <= BANKING_MODE_END) {
        banking_mode = val & 0x01;
    }
    else if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END) {
        if (!banking_mode) {
            ram[addr - SWITCHABLE_RAM_START] = val;
        } else {
            ram[current_ram_bank * SWITCHABLE_RAM_SIZE + (addr - SWITCHABLE_RAM_START)] = val;
        }
    }
}