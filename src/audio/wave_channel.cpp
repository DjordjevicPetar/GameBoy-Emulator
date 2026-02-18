#include "wave_channel.hpp"

WaveChannel::WaveChannel() {
    reset();
}

void WaveChannel::reset() {
    nr30 = 0;
    nr31 = 0;
    nr32 = 0;
    nr33 = 0;
    nr34 = 0;

    enabled = false;
    dac_enabled = false;
    
    frequency = 0;
    timer = 0;

    wave_position = 0;

    length_timer = 0;
    length_enabled = false;

    for (int pos = 0; pos < 16; pos++) {
        wave_ram[pos] = 0;
    }
}

u8 WaveChannel::read_nr30() {
    return nr30 | 0x7F;
}

u8 WaveChannel::read_nr31() {
    return 0xFF;
}

u8 WaveChannel::read_nr32() {
    return nr32 | 0x9F;
}

u8 WaveChannel::read_nr33() {
    return 0xFF;
}

u8 WaveChannel::read_nr34() {
    return nr34 | 0xBF;
}

u8 WaveChannel::read_wave_ram(u16 addr) {
    // TODO: When CH3 is active (enabled and DAC on), reading wave RAM should return
    // the byte at the current wave_position, not the addressed byte. The current
    // implementation allows free access regardless of channel state.
    return wave_ram[addr & 0x0F];
}

void WaveChannel::write_nr30(u8 val) {
    nr30 = val;

    dac_enabled = (val & 0x80) != 0;

    if (!dac_enabled) {
        enabled = false;
    }
}

void WaveChannel::write_nr31(u8 val) {
    nr31 = val;

    length_timer = 256 - val;
}

void WaveChannel::write_nr32(u8 val) {
    nr32 = val;

    output_level = (val >> 5) & 0x03;
}

void WaveChannel::write_nr33(u8 val) {
    nr33 = val;

    frequency = (frequency & 0x0700) | val;
}

void WaveChannel::write_nr34(u8 val) {
    nr34 = val;

    frequency = (frequency & 0x00FF) | ((val & 0x07) << 8);
    length_enabled = (val & 0x40) != 0;

    if (val & 0x80) {
        trigger();
    }
}

void WaveChannel::write_wave_ram(u16 addr, u8 val) {
    wave_ram[addr & 0x0F] = val;
}

void WaveChannel::trigger() {
    enabled = dac_enabled;

    if (length_timer == 0) {
        length_timer = 256;
    }

    wave_position = 0;

    timer = (2048 - frequency) * 2;
}

void WaveChannel::step(u8 cycles) {
    if (!enabled) return;

    timer -= cycles;

    while (timer <= 0) {
        timer += (2048 - frequency) * 2;
        wave_position = (wave_position + 1) % 32;
    }
}

void WaveChannel::clock_sound_length() {
    if (!length_enabled) return;

    if (length_timer > 0) {
        length_timer--;
    }

    if (length_timer == 0) {
        enabled = false;
        return;
    }
}

u8 WaveChannel::output() {
    if (!enabled || !dac_enabled) return 0;

    u8 byte = wave_ram[wave_position / 2];
    u8 sample;

    // TODO(bug): Nibble order is swapped. The Game Boy plays the HIGH nibble first
    // (even positions) then the LOW nibble (odd positions). Currently even positions
    // read the low nibble and odd positions read the high nibble - this is backwards.
    // Fix: swap the branches (even -> byte >> 4, odd -> byte & 0x0F).
    if (wave_position % 2) {
        sample = byte >> 4;
    } else {
        sample = byte & 0x0F;
    }

    switch (output_level) {
        case 0: return 0;
        case 1: return sample;
        case 2: return sample >> 1;
        case 3: return sample >> 2;
    }

    return 0;
}