#include "apu.hpp"

APU::APU() :
    ch1(true),
    ch2(false),
    ch3(),
    ch4()
{
    enabled = false;
    cycle_counter = 0;
    frame_counter = 0;

    nr50 = 0;
    nr51 = 0;
    nr52 = 0;

    ch1.reset();
    ch2.reset();
    ch3.reset();
    ch4.reset();
}

void APU::reset() {
    nr50 = 0;
    nr51 = 0;
    nr52 = 0;

    ch1.reset();
    ch2.reset();
    ch3.reset();
    ch4.reset();
}

void APU::step(u8 cycles) {
    cycle_counter += cycles;
    // Frame squencer works every 8192 CPU instructions (512Hz)
    if (cycle_counter > 8192) {
        cycle_counter -= 8192;

        frame_counter = (frame_counter + 1) % 8;

        switch (frame_counter) {
            case 0:
                ch1.clock_sound_length();
                ch2.clock_sound_length();
                ch3.clock_sound_length();
                ch4.clock_sound_length();
                break;
            case 2:
                ch1.clock_sound_length();
                ch2.clock_sound_length();
                ch3.clock_sound_length();
                ch4.clock_sound_length();
                ch1.clock_sweep();
                break;
            case 4:
                ch1.clock_sound_length();
                ch2.clock_sound_length();
                ch3.clock_sound_length();
                ch4.clock_sound_length();
                break;
            case 6:
                ch1.clock_sound_length();
                ch2.clock_sound_length();
                ch3.clock_sound_length();
                ch4.clock_sound_length();
                ch1.clock_sweep();
                break;
            case 7:
                ch1.clock_envelope();
                ch2.clock_envelope();
                ch4.clock_envelope();
                break;
        }
    }

    ch1.step(cycles);
    ch2.step(cycles);
    ch3.step(cycles);
    ch4.step(cycles);
}

void APU::write_nr50(u8 val) {
    nr50 = val;
}

void APU::write_nr51(u8 val) {
    nr51 = val;
}

void APU::write_nr52(u8 val) {
    nr52 = nr52 | (val & 0x80);

    bool new_enabled = val & 0x80;
    bool old_enabled = enabled;

    enabled = new_enabled;

    if (!enabled && old_enabled) {
        reset();
    }
}

u8 APU::read(u16 addr) {
    switch (addr) {
        // Channel 1 - Square Channel 1
        case 0xFF10: return ch1.read_nrx0();
        case 0xFF11: return ch1.read_nrx1();
        case 0xFF12: return ch1.read_nrx2();
        case 0xFF13: return ch1.read_nrx3();
        case 0xFF14: return ch1.read_nrx4();

        // Channel 2 - Square Channel 2; doesn't have frequency sweep
        case 0xFF16: return ch2.read_nrx1();
        case 0xFF17: return ch2.read_nrx2();
        case 0xFF18: return ch2.read_nrx3();
        case 0xFF19: return ch2.read_nrx4();

        // Channel 3 - Wave Channel
        case 0xFF1A: return ch3.read_nr30();
        case 0xFF1B: return ch3.read_nr31();
        case 0xFF1C: return ch3.read_nr32();
        case 0xFF1D: return ch3.read_nr33();
        case 0xFF1E: return ch3.read_nr34();

        // Channel 4 - Noise Channel
        case 0xFF20: return ch4.read_nr41();
        case 0xFF21: return ch4.read_nr42();
        case 0xFF22: return ch4.read_nr43();
        case 0xFF23: return ch4.read_nr44();
    }

    if (addr >= 0xFF30 && addr <= 0xFF3F) {
        return ch3.read_wave_ram(addr - 0xFF30);
    }

    return DEFAULT_READ_RETURN;
}

void APU::write(u16 addr, u8 val) {
    // If not enabled write not allowed (expect for NR52 register)
    if (!enabled && addr != 0xFF26) return;

    switch (addr) {
        // Channel 1
        case 0xFF10: ch1.write_nrx0(val); break; 
        case 0xFF11: ch1.write_nrx1(val); break; 
        case 0xFF12: ch1.write_nrx2(val); break; 
        case 0xFF13: ch1.write_nrx3(val); break; 
        case 0xFF14: ch1.write_nrx4(val); break; 

        // Channel 2 - doesn't have sweep (nrx0)
        case 0xFF16: ch2.write_nrx1(val); break; 
        case 0xFF17: ch2.write_nrx2(val); break; 
        case 0xFF18: ch2.write_nrx3(val); break; 
        case 0xFF19: ch2.write_nrx4(val); break; 

        // Channel 3
        case 0xFF1A: ch3.write_nr30(val); break;
        case 0xFF1B: ch3.write_nr31(val); break;
        case 0xFF1C: ch3.write_nr32(val); break;
        case 0xFF1D: ch3.write_nr33(val); break;
        case 0xFF1E: ch3.write_nr34(val); break;

        // Channel 4
        case 0xFF20: ch4.write_nr41(val); break;
        case 0xFF21: ch4.write_nr42(val); break;
        case 0xFF22: ch4.write_nr43(val); break;
        case 0xFF23: ch4.write_nr44(val); break;

        case 0xFF24: write_nr50(val); break; // Master volume & VIN panning
        case 0xFF25: write_nr51(val); break; // Sound panning
        case 0xFF26: write_nr52(val); break; // Audio master control
    }

    if (addr >= 0xFF30 && addr <= 0xFF3F) {
        ch3.write_wave_ram(addr - 0xFF30, val);
    }
}

AudioSample APU::mix() {
    AudioSample out = {0, 0};
    int left = 0;
    int right = 0;

    if (!(nr52 & 0x80)) return out;

    int ch1_out = (int)ch1.output() - 8;
    int ch2_out = (int)ch2.output() - 8;
    int ch3_out = (int)ch3.output() - 8;
    int ch4_out = (int)ch4.output() - 8;

    if (nr51 & 0x01) right += ch1_out;
    if (nr51 & 0x02) right += ch2_out;
    if (nr51 & 0x04) right += ch3_out;
    if (nr51 & 0x08) right += ch4_out;

    if (nr51 & 0x10) left += ch1_out;
    if (nr51 & 0x20) left += ch2_out;
    if (nr51 & 0x40) left += ch3_out;
    if (nr51 & 0x80) left += ch4_out;

    right = (right * ((nr50 & 0x07) + 1)) / 8;
    left = (left * (((nr50 >> 4) & 0x07) + 1)) / 8;

    out.left = left * 512;
    out.right = right * 512;

    return out;
}