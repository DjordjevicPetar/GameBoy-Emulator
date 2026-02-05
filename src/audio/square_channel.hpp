#pragma once

#include "../memory/memory_constants.hpp"

class SquareChannel {
public:
    SquareChannel();
    void reset();

    void step(u8 cycles);
    
    u8 read_nrx0();
    u8 read_nrx1();
    u8 read_nrx2();
    u8 read_nrx3();
    u8 read_nrx4();

    void write_nrx0(u8 val);
    void write_nrx1(u8 val);
    void write_nrx2(u8 val);
    void write_nrx3(u8 val);
    void write_nrx4(u8 val);
private:
    u8 nrx0;
    u8 nrx1;
    u8 nrx2;
    u8 nrx3;
    u8 nrx4;
};