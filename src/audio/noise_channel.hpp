#pragma once

#include "../utils/types.hpp"

class NoiseChannel {
public:
    NoiseChannel();
    void reset();

    void step(u8 cycles);

    u8 read_nr41();
    u8 read_nr42();
    u8 read_nr43();
    u8 read_nr44();

    void write_nr41(u8 val);
    void write_nr42(u8 val);
    void write_nr43(u8 val);
    void write_nr44(u8 val);

    void clock_envelope();
    void clock_sound_length();

    u8 output();
private:
    u8 nr41, nr42, nr43, nr44;

    bool enabled;
    bool dac_enabled;

    int timer;

    u8 env_timer;
    u8 current_volume;

    u16 lfsr;

    // nr41:
    u16 length_timer;

    // nr42:
    u8 env_volume;
    u8 env_increase;
    u8 env_pace;

    // nr43:
    u8 clock_shift;
    bool lfsr_width;
    u8 clock_divider;

    // nr44:
    bool length_enabled;

    void trigger();
};