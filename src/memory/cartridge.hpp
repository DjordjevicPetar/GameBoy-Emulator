#pragma once

#include "../utils/types.hpp"


#include "../memory/memory_constants.hpp"
#include "mbc.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>
#include <memory>

class Cartridge {
public:
    Cartridge(std::string path);

    bool load_rom(std::string path);
    void print_rom();

    void parse_header();

    u8 read8(u16 addr) const;
    void write8(u16 addr, u8 val);
private:
    std::vector<u8> rom;
    int rom_banks;

    std::vector<u8> ram;
    int ram_banks;

    u8 cartridge_type;
    std::unique_ptr<MBC> mbc;
};

