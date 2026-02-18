#include "noise_channel.hpp"

NoiseChannel::NoiseChannel() {
    reset();
}

void NoiseChannel::reset() {
    nr41 = 0;
    nr42 = 0;
    nr43 = 0;
    nr44 = 0;

    enabled = false;

    timer = 0;

    length_timer = 0;

    env_timer = 0;
    env_pace = 0;
    env_increase = 0;
    env_volume = 0;
    current_volume = 0;

    clock_shift = 0;
    lfsr_width = false;
    clock_divider = 0;
}

u8 NoiseChannel::read_nr41() {
    return 0xFF;
}

u8 NoiseChannel::read_nr42() {
    return nr42;
}
u8 NoiseChannel::read_nr43() {
    return 0xFF;
}
u8 NoiseChannel::read_nr44() {
    return nr44 | 0xBF;
}

void NoiseChannel::write_nr41(u8 val) {
    nr41 = val;

    length_timer = 64 - (val & 0x3F);
}

void NoiseChannel::write_nr42(u8 val) {
    nr42 = val;

    env_volume = (val >> 4) & 0x0F;
    env_increase = (val >> 3) & 0x01;
    env_pace = val & 0x07;

    dac_enabled = (val & 0xF8) != 0;

    if (!dac_enabled) {
        enabled = false;
    }
}
void NoiseChannel::write_nr43(u8 val) {
    nr43 = val;

    clock_shift = (val >> 4) & 0x0F;
    // TODO(bug): LFSR width is bit 3 of NR43, not bit 7. Bit 7 is part of clock_shift.
    // NR43 layout: bits 7-4 = clock shift, bit 3 = width mode, bits 2-0 = divider.
    // Fix: (val & 0x08) != 0
    lfsr_width = (val & 0x08) != 0;
    clock_divider = val & 0x07;
}
void NoiseChannel::write_nr44(u8 val) {
    nr44 = val;

    length_enabled = (val & 0x40) != 0;

    if (val & 0x80) {
        trigger();
    }
}

void NoiseChannel::trigger() {
    enabled = dac_enabled;

    if (length_timer == 0) {
        length_timer = 64;
    }

    current_volume = env_volume;
    env_timer = (env_pace == 0) ? 8 : env_pace;

    lfsr = 0x7FFF;
    
    timer = divisors[clock_divider] << clock_shift;
}

void NoiseChannel::step(u8 cycles) {
    if (!enabled) return;

    timer -= cycles;

    while (timer <= 0) {
        timer = divisors[clock_divider] << clock_shift;

        u8 resulting_bit = (lfsr & 0x01) ^ ((lfsr >> 1) & 0x01);

        lfsr >>= 1;
        lfsr |= resulting_bit << 14;

        if (lfsr_width) {
        // TODO(bug): The mask (~1 << 6) is incorrect due to operator precedence.
        // ~1 is 0xFFFFFFFE (all bits set except bit 0), then << 6 gives 0xFFFFFF80,
        // which clears bits 0-6 instead of just bit 6. Should be: ~(1 << 6).
        lfsr &= ~(1 << 6);
        lfsr |= resulting_bit << 6;
        }
    }
}

void NoiseChannel::clock_envelope() {
    if (env_pace == 0) return;

    if (env_timer > 0) {
        env_timer--;
    }

    if (env_timer == 0) {
        env_timer = (env_pace == 0) ? 8 : env_pace;

        if (env_increase) {
            if (current_volume < 15) current_volume++;
        } else {
            if (current_volume > 0) current_volume--;
        }
    }
}

void NoiseChannel::clock_sound_length() {
    if (!length_enabled) return;

    if (length_timer > 0) {
        length_timer--;
    }

    if (length_timer == 0) {
        enabled = false;
        return;
    }
}

u8 NoiseChannel::output() {
    if (!enabled || !dac_enabled) return 0;

    return (~lfsr & 1) ? current_volume : 0;
}