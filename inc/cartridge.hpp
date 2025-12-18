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

    bool loadROM(string path);
    void printROM();

    uint8_t read8(uint16_t addr);
    void write8(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    int currentBank;
};

#endif