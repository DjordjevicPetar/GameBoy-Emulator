#pragma once

#include "../utils/types.hpp"


#include <vector>
#include <iostream>
#include "../memory/memory_constants.hpp"

class MBC {
public:
    virtual ~MBC() = default;
    virtual u8 read(u16 addr) = 0;
    virtual void write(u16 addr, u8 val) = 0;
};

class MBC0 : public MBC {
public:
    MBC0(std::vector<u8>& rom, std::vector<u8>& ram);
    u8 read(u16 addr);
    void write(u16 addr, u8 val);
private:
    std::vector<u8> rom;
    std::vector<u8> ram;
};

class MBC1 : public MBC {
public:
    MBC1(std::vector<u8>& rom, std::vector<u8>& ram);
    u8 read(u16 addr);
    void write(u16 addr, u8 val);
private:
    std::vector<u8> rom;
    std::vector<u8> ram;

    u8 current_rom_bank_low;
    u8 current_rom_bank_high;
    int rom_banks;

    u8 current_ram_bank;
    bool ram_enabled;
    
    bool banking_mode;
};

