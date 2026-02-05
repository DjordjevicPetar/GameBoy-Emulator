#include "square_channel.hpp"

SquareChannel::SquareChannel(bool has_sweep) :
    has_sweep(has_sweep)
{}

u8 SquareChannel::read_nrx0() {
    return nrx0;
}

u8 SquareChannel::read_nrx1() {
    return nrx1;
}

u8 SquareChannel::read_nrx2() {
    return nrx2;
}

u8 SquareChannel::read_nrx3() {
    return nrx3;
}

u8 SquareChannel::read_nrx4() {
    return nrx4;
}

void SquareChannel::write_nrx0(u8 val) {
    if (!has_sweep) return;

    nrx0 = val;

    sweep_pace = (val >> 4) & 0x07;
    sweep_negate = val & 0x08;
    sweep_step = val & 0x07;
}

void SquareChannel::write_nrx1(u8 val) {
    nrx1 = val;

    wave_duty = (val >> 6) & 0x03;
    length_timer = 64 - (val & 0x3F);
}

void SquareChannel::write_nrx2(u8 val) {
    nrx2 = val;

    env_volume = (val >> 4) & 0x0F;
    env_increase = val & 0x08;
    env_pace = val & 0x07;
}

void SquareChannel::write_nrx3(u8 val) {
    nrx3 = val;

    period = (period & 0x0700) | val;
}

void SquareChannel::write_nrx4(u8 val) {
    nrx4 = val;

    trigger_bit = val & 0x80;
    period = (period & 0xFF) | ((val & 0x07) << 8);
    length_enabled = val & 0x40;
    
    if (trigger_bit) trigger();
}

void SquareChannel::trigger() {
    enabled = true;

    if (length_timer == 0) {
        length_timer = 64;
    }

    period_divider = (2048 - period) * 4;

    env_timer = env_pace;
    current_volume = env_volume;

    if (has_sweep) {
        shadow_period = period;

        sweep_timer = (sweep_pace == 0) ? 8 : sweep_pace;

        sweep_enabled = sweep_pace != 0 || sweep_step != 0;

        if (sweep_step > 0) {
            // TODO: If the individual step is non-zero,
            // frequency calculation and overflow check are performed immediately.
        }
    }
}