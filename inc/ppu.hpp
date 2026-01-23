#ifndef _PPU_HPP_
#define _PPU_HPP_

#include <cstdint>
#include <vector>
#include "constants_mmu.hpp"
#include "interrupt_controller.hpp"
#include <iostream>

enum PPUMode {
    HBlank, VBlank, OAM, DRAW
};

class PPU {
public:
    PPU(InterruptController* interrupt_controller);
    void reset();
    void step(uint8_t cycles);

    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t val);

    const std::vector<std::vector<uint32_t>>& get_framebuffer() const;

    PPUMode get_mode();
    uint8_t get_ly();

private:
    InterruptController* interrupt_controller;

    std::vector<std::vector<uint32_t>> framebuffer;
    std::vector<uint8_t> vram;
    std::vector<uint8_t> oam;
    std::vector<uint32_t> palette;

    PPUMode mode;
    int cycle_counter;

    void render_scanline();

    void render_background();
    void render_window();
    void render_sprites();

    void update_stat_register();
    void check_lyc_interrupt();

    void clear_framebuffer();

    uint8_t lcdc;
    uint8_t stat;
    uint8_t scy;
    uint8_t scx;
    uint8_t ly;
    uint8_t lyc;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wy;
    uint8_t wx;

    std::vector<uint32_t> line;
};

#endif