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

    void parseHeader(); // TODO
    void allocateRAM(); // TODO

    uint8_t read8(uint16_t addr);
    void write8(uint16_t addr, uint8_t val);

    uint8_t readRAM(uint16_t addr);
    void writeRAM(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    vector<uint8_t> ram;
    int currentBank;
};

#endif