#pragma once

#include "../utils/types.hpp"

class WaveChannel {
public:
    WaveChannel();
    void reset();

    void step(u8 cycles);

    u8 read_nr30();
    u8 read_nr31();
    u8 read_nr32();
    u8 read_nr33();
    u8 read_nr34();
    u8 read_wave_ram(u16 addr);

    void write_nr30(u8 val);
    void write_nr31(u8 val);
    void write_nr32(u8 val);
    void write_nr33(u8 val);
    void write_nr34(u8 val);
    void write_wave_ram(u16 addr, u8 val);

    void clock_sound_length();

    u8 output();
    
    bool is_enabled();
private:
    u8 nr30, nr31, nr32, nr33, nr34;

    bool enabled;

    int timer;

    u8 wave_ram[16];
    u8 wave_position;

    // nr30:
    bool dac_enabled;

    // nr31:
    u16 length_timer;

    // nr32:
    u8 output_level;

    // nr33:
    u16 frequency; // frequency[10:0] = nrx4[2:0]nrx3[7:0]

    // nr34:
    bool length_enabled;

    void trigger();
};