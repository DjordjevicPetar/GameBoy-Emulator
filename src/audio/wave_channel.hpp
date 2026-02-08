#pragma once

class WaveChannel {
public:
    WaveChannel() {}
    void reset() {}

    void step(u8 cycles) {}

    u8 read_nr30();
    u8 read_nr31();
    u8 read_nr32();
    u8 read_nr33();
    u8 read_nr34();

    void write_nr30(u8 val);
    void write_nr31(u8 val);
    void write_nr32(u8 val);
    void write_nr33(u8 val);
    void write_nr34(u8 val);
private:
    u8 nr30, nr31, nr32, nr33, nr34;

    u8 wave_ram[16];
};