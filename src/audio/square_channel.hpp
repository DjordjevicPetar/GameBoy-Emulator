#pragma once

#include "../memory/memory_constants.hpp"

class SquareChannel {
public:
    SquareChannel(bool has_sweep);
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
    bool has_sweep;

    u8 nrx0, nrx1, nrx2, nrx3, nrx4;

    bool enabled;
    bool dac_enabled;

    u16 period_divider;

    u8 env_timer;
    u8 current_volume;

    u8 duty_pos;

    u8 sweep_timer;
    bool sweep_enabled;
    
    u8 shadow_period;
    
    // nrx0:
    u8 sweep_pace;
    bool sweep_negate;
    u8 sweep_step;

    // nrx1:
    u8 wave_duty;
    u8 length_timer;

    // nrx2:
    u8 env_volume;
    u8 env_increase;
    u8 env_pace;

    // nrx3:
    u16 period; // lower 8 bits from nrx3[7:0], higher 3 bits from nrx4[2:0]

    // nrx4:
    bool length_enabled;
    bool trigger_bit;
    void trigger();
};