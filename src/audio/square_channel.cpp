#include "square_channel.hpp"

SquareChannel::SquareChannel(bool has_sweep) :
    has_sweep(has_sweep)
{}

u8 SquareChannel::read_nrx0() {
    return nrx0 | 0x80;
}

u8 SquareChannel::read_nrx1() {
    return nrx1 | 0x3F;
}

u8 SquareChannel::read_nrx2() {
    return nrx2;
}

u8 SquareChannel::read_nrx3() {
    return 0xFF;
}

u8 SquareChannel::read_nrx4() {
    return nrx4 | 0xBF; 
}

void SquareChannel::write_nrx0(u8 val) {
    if (!has_sweep) return;

    nrx0 = val;

    sweep_pace = (val >> 4) & 0x07;
    sweep_negate = (val >> 3) & 0x01;
    sweep_step = val & 0x07;
    
    sweep_enabled = sweep_pace != 0 || sweep_step != 0;
}

void SquareChannel::write_nrx1(u8 val) {
    nrx1 = val;

    wave_duty = (val >> 6) & 0x03;
    length_timer = 64 - (val & 0x3F);
}

void SquareChannel::write_nrx2(u8 val) {
    nrx2 = val;

    env_volume = (val >> 4) & 0x0F;
    env_increase = (val >> 3) & 0x01;
    env_pace = val & 0x07;

    dac_enabled = (val & 0xF8) != 0;

    if (!dac_enabled) {
        enabled = false;
    }
}

void SquareChannel::write_nrx3(u8 val) {
    nrx3 = val;

    period = (period & 0x0700) | val;
}

void SquareChannel::write_nrx4(u8 val) {
    nrx4 = val;

    period = (period & 0xFF) | ((val & 0x07) << 8);
    length_enabled = (val & 0x40) != 0;
    
    if (val & 0x80) {
        trigger();
    }
}

void SquareChannel::trigger() {
    enabled = true;

    if (length_timer == 0) {
        length_timer = 64;
    }

    timer = (2048 - period) * 4;

    env_timer = (env_pace == 0) ? 8 : env_pace;
    current_volume = env_volume;

    if (has_sweep) {
        shadow_register = period;

        sweep_timer = (sweep_pace == 0) ? 8 : sweep_pace;

        sweep_enabled = sweep_pace != 0 || sweep_step != 0;

        if (sweep_step > 0) {
            u16 new_freq = calculate_sweep_new_frequency();

            if (new_freq > 2047) {
                enabled = false;
            }
        }
    }
}

u16 SquareChannel::calculate_sweep_new_frequency() {
    if (sweep_negate) {
        return shadow_register - (shadow_register >> sweep_step);
    } else {
        return shadow_register + (shadow_register >> sweep_step);
    }
}