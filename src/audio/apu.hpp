#pragma once

#include "../memory/memory_constants.hpp"
#include "square_channel.hpp"
#include "wave_channel.hpp"
#include "noise_channel.hpp"

class APU {
public:
    APU();
    void reset();
    
    u8 read(u16 addr);
    void write(u16 addr, u8 val);

    void step(u8 cycles);

private:
    SquareChannel ch1;
    SquareChannel ch2;
    WaveChannel ch3;
    NoiseChannel ch4;

    bool enabled;

    u16 cycle_counter;
    u8 frame_counter;

    u8 nr52, nr51, nr50;

    void write_nr50(u8 val);
    void write_nr51(u8 val);
    void write_nr52(u8 val);
};