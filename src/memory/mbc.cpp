#include "mbc.hpp"

MBC0::MBC0(std::vector<u8>& rom, std::vector<u8>& ram) : rom(rom), ram(ram) {}

u8 MBC0::read(u16 addr) { // 2 ROM banks
    if (addr <= SWITCHABLE_ROM_END) {
        if (addr < rom.size()) {
            return rom[addr];
        }
        return DEFAULT_READ_RETURN;
    }
    else if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END && !ram.empty()) {
        size_t offset = addr - SWITCHABLE_RAM_START;
        if (offset < ram.size()) {
            return ram[offset];
        }
        return DEFAULT_READ_RETURN;
    }
    return DEFAULT_READ_RETURN;
}

void MBC0::write(u16 addr, u8 val) {
    if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END) {
        size_t offset = addr - SWITCHABLE_RAM_START;
        if (offset < ram.size()) {
            ram[offset] = val;
        }
    }
}

MBC1::MBC1(std::vector<u8>& rom, std::vector<u8>& ram) : rom(rom), ram(ram) {
    rom_banks = rom.size() / SWITCHABLE_ROM_SIZE;
    ram_enabled = false;
    banking_mode = false;
    current_rom_bank_low = 1;
    current_rom_bank_high = 0;
    current_ram_bank = 0;
}
 
u8 MBC1::read(u16 addr) {
    if (addr <= STATIC_ROM_END) {
        u8 bank = 0;

        if (banking_mode == 1) {
            bank = current_rom_bank_high << 5;
        }
        bank %= rom_banks;

        size_t offset = bank * STATIC_ROM_SIZE + (addr - STATIC_ROM_START);
        if (offset < rom.size()) {
            return rom[offset]; 
        }
        return DEFAULT_READ_RETURN;
    }
    else if (addr <= SWITCHABLE_ROM_END) {
        u8 bank = 0;

        bank = current_rom_bank_high << 5 | current_rom_bank_low;
        bank %= rom_banks;
        if ((bank & MBC1_ROM_BANKS_MASK) == 0) {
            bank += 1;
        }

        size_t offset = bank * SWITCHABLE_ROM_SIZE + (addr - SWITCHABLE_ROM_START);
        if (offset < rom.size()) {
            return rom[offset]; 
        }
        return DEFAULT_READ_RETURN;
    }
    else if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END && ram_enabled) {
        u8 bank = (banking_mode == 0) ? 0 : current_ram_bank;
        
        size_t offset = bank * SWITCHABLE_RAM_SIZE + (addr - SWITCHABLE_RAM_START);
        if (offset < ram.size()) {
            return ram[offset]; 
        }
        return DEFAULT_READ_RETURN;
    }
    return DEFAULT_READ_RETURN; 
}

void MBC1::write(u16 addr, u8 val) {
    if (addr <= RAM_ENABLE_END) {
        ram_enabled = (val & MBC1_RAM_ENABLE_MASK) == MBC1_RAM_ENABLE_ENABLED;
    }
    else if (addr <= ROM_BANK_SELECT_END) {
        // TODO: The 0->1 fixup should NOT be done here at write time. It should be
        // done at read time when computing the effective bank number. Writing 0x00
        // to this register stores 0x00; the bank calculation then treats 0 as 1.
        // Doing it here means reading the register back returns 1 instead of 0,
        // and it can interact incorrectly with the high bits for large ROMs.
        current_rom_bank_low = val & MBC1_ROM_BANKS_MASK;
        if (current_rom_bank_low == 0) current_rom_bank_low = 1;
    }
    else if (addr <= RAM_BANK_SELECT_END) {
        current_rom_bank_high = val & MBC1_RAM_BANKS_MASK;
        current_ram_bank = val & MBC1_RAM_BANKS_MASK;
    }
    else if (addr <= BANKING_MODE_END) {
        banking_mode = val & 0x01;
    }
    else if (addr >= SWITCHABLE_RAM_START && addr <= SWITCHABLE_RAM_END && ram_enabled) {
        u8 bank = (banking_mode == 0) ? 0 : current_ram_bank;
        
        size_t offset = bank * SWITCHABLE_RAM_SIZE + (addr - SWITCHABLE_RAM_START);
        if (offset < ram.size()) {
            ram[offset] = val;
        }
    }
}