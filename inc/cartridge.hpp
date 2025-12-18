#ifndef _CARTRIDGE_HPP_
#define _CARTRIDGE_HPP_

#include "constants.hpp"
#include <iostream>
#include <vector>

using namespace std;

class Cartridge {
public:
    bool loadROM(string path);

    uint8_t read8(uint16_t addr);
    void write8(uint16_t addr, uint8_t val);
private:
    vector<uint8_t> rom;
    int currentBank;
};

#endif