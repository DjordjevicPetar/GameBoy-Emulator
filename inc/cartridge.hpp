#ifndef _CARTRIDGE_HPP_
#define _CARTRIDGE_HPP_

#include "constants_mmu.hpp"
#include "mbc.hpp"
#include <iostream>
#include <cstdint>
#include <vector>
#include <fstream>
#include <iomanip>
#include <memory>

using namespace std;

class Cartridge {
public:
    Cartridge(std::string path);

    bool load_rom(string path);
    void print_rom();

    void parse_header(); // TODO

    uint8_t read8(uint16_t addr) const;
    void write8(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    int rom_banks;

    vector<uint8_t> ram;
    int ram_banks;

    uint8_t cartridge_type;
    unique_ptr<MBC> mbc;
};

#endif