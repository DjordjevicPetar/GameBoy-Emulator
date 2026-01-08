#ifndef _MBC_HPP_
#define _MBC_HPP_

#include <cstdint>
#include <vector>
#include <iostream>
#include "constants_mmu.hpp"

class MBC {
public:
    virtual ~MBC() = default;
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t val) = 0;
};

class MBC0 : public MBC {
public:
    MBC0(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;
};

class MBC1 : public MBC {
public:
    MBC1(std::vector<uint8_t>& rom, std::vector<uint8_t>& ram);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
private:
    std::vector<uint8_t> rom;
    std::vector<uint8_t> ram;

    uint8_t current_rom_bank_low;
    uint8_t current_rom_bank_high;
    int rom_banks;

    uint8_t current_ram_bank;
    bool ram_enabled;
    
    bool banking_mode;
};

#endif