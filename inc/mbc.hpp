#ifndef _MBC_HPP_
#define _MBC_HPP_

#include <cstdint>
#include <vector>
#include "constants_mmu.hpp"

using namespace std;

class MBC {
public:
    virtual uint8_t read(uint16_t addr) = 0;
    virtual void write(uint16_t addr, uint8_t val) = 0;
private:
};

class MBC0 : MBC {
public:
    MBC0(vector<uint8_t>& rom, vector<uint8_t>& ram);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    vector<uint8_t> ram;
};

class MBC1 : MBC {
public:
    MBC1(vector<uint8_t>& rom, vector<uint8_t>& ram);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    vector<uint8_t> ram;

    uint8_t current_rom_bank_low;
    int rom_banks;

    uint8_t current_rom_bank_high;

    uint8_t current_ram_bank;
    bool ram_enabled;
    
    bool banking_mode;
};

#endif