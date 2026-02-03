#include "apu.hpp"

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
    switch (addr) {
        // Channel 1
        case 0xFF10: ch1.write_nrx0(val);
        case 0xFF11: ch1.write_nrx1(val);
        case 0xFF12: ch1.write_nrx2(val);
        case 0xFF13: ch1.write_nrx3(val);
        case 0xFF14: ch1.write_nrx4(val);

        // Channel 2 - doesn't have sweep (nrx0)
        case 0xFF16: ch2.write_nrx1(val);
        case 0xFF17: ch2.write_nrx2(val);
        case 0xFF18: ch2.write_nrx3(val);
        case 0xFF19: ch2.write_nrx4(val);
    }
}