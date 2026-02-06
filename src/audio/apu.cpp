#include "apu.hpp"

APU::APU() :
    ch1(true),
    ch2(false),
    ch3(),
    ch4()
{
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
    if (cycle_counter > 8192) {
        cycle_counter -= 8192;
        // TODO: Handle 
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
    if (!((nr52 & 0x80) >> 7)) reset();
}

u8 APU::read(u16 addr) {
    switch (addr) {
        // Channel 1
        case 0xFF10: return ch1.read_nrx0();
        case 0xFF11: return ch1.read_nrx1();
        case 0xFF12: return ch1.read_nrx2();
        case 0xFF13: return ch1.read_nrx3();
        case 0xFF14: return ch1.read_nrx4();

        // Channel 2 - doesn't have sweep (nrx0)
        case 0xFF16: return ch2.read_nrx1();
        case 0xFF17: return ch2.read_nrx2();
        case 0xFF18: return ch2.read_nrx3();
        case 0xFF19: return ch2.read_nrx4();
    }
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

        // ...

        case 0xFF24: write_nr50(val); break; // Master volume & VIN panning
        case 0xFF25: write_nr51(val); break; // Sound panning
        case 0xFF26: write_nr52(val); break; // Audio master control
    }
}