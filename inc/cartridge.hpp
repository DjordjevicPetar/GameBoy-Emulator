#ifndef _CARTRIDGE_HPP_
#define _CARTRIDGE_HPP_

#include "constants_mmu.hpp"
#include <iostream>
#include <cstdint>
#include <vector>
#include <fstream>
#include <iomanip>

using namespace std;

class Cartridge {
public:
    Cartridge();

    bool load_rom(string path);
    void print_rom();

    void parse_header(); // TODO
    void allocate_ram(); // TODO

    uint8_t read8_rom(uint16_t addr) const;
    void write8_rom(uint16_t addr, uint8_t val);

    uint8_t read8_ram(uint16_t addr) const;
    void write8_ram(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    int rom_banks;
    vector<uint8_t> ram;
    int ram_banks;
    int current_bank;
};

#endif