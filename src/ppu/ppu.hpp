#pragma once

#include "utils/types.hpp"


#include <vector>
#include "memory/memory_constants.hpp"
#include "interrupt/interrupt_controller.hpp"
#include <iostream>

// PPU Register Addresses
enum PPU_REGISTER_ADDRESSES {
    PPU_REGISTER_LCDC = 0xFF40,
    PPU_REGISTER_STAT = 0xFF41,
    PPU_REGISTER_SCY = 0xFF42,
    PPU_REGISTER_SCX = 0xFF43,
    PPU_REGISTER_LY = 0xFF44,
    PPU_REGISTER_LYC = 0xFF45,
    PPU_REGISTER_DMA = 0xFF46,
    PPU_REGISTER_BGP = 0xFF47,
    PPU_REGISTER_OBP0 = 0xFF48,
    PPU_REGISTER_OBP1 = 0xFF49,
    PPU_REGISTER_WY = 0xFF4A,
    PPU_REGISTER_WX = 0xFF4B,
};

// PPU LCDC Register Masks
enum PPU_LCDC_MASKS {
    LCDC_BG_AND_WIN_ENABLE_MASK = 0x01,
    LCDC_OBJ_ENABLE_MASK = 0x02,
    LCDC_OBJ_SIZE_MASK = 0x04,
    LCDC_BG_TILE_MAP_AREA_MASK = 0x08,
    LCDC_BG_AND_WIN_TILE_DATA_AREA_MASK = 0x10,
    LCDC_WIN_ENABLE_MASK = 0x20,
    LCDC_WIN_TILE_MAP_AREA_MASK = 0x40,
    LCDC_PPU_ENABLE_MASK = 0x80,
};

// PPU STAT Register Masks
enum PPU_STAT_MASKS {
    STAT_MODE_MASK = 0x03,
    STAT_LY_COMPARE_FALSE = 0x00,
    STAT_LY_COMPARE_TRUE = 0x04,
    STAT_READ_ONLY_MASK = 0x07,
    STAT_HBLANK_INTERRUPT_ENABLE_MASK = 0x08,
    STAT_VBLANK_INTERRUPT_ENABLE_MASK = 0x10,
    STAT_OAM_INTERRUPT_ENABLE_MASK = 0x20,
    STAT_LYC_INTERRUPT_ENABLE_MASK = 0x40,
    STAT_READ_WRITE_MASK = 0x78,
};

// PPU Timing (cycles)
enum PPU_TIMING {
    PPU_OAM_CYCLES = 80,
    PPU_DRAW_CYCLES = 172,
    PPU_FULL_LINE_CYCLES = 456,
    PPU_HBLANK_CYCLES = PPU_FULL_LINE_CYCLES - PPU_DRAW_CYCLES - PPU_OAM_CYCLES,
};

// PPU Scanlines
enum PPU_SCANLINES {
    PPU_VBLANK_FIRST_LINE = 144,
    PPU_VBLANK_LAST_LINE = 153,
};

// PPU Background Tile Map Addresses
enum PPU_TILE_MAP_ADDRESSES {
    BACKGROUND_TILE_MAP0_START = 0x9800,
    BACKGROUND_TILE_MAP0_END = 0x9BFF,
    BACKGROUND_TILE_MAP1_START = 0x9C00,
    BACKGROUND_TILE_MAP1_END = 0x9FFF,
};

// PPU Palette Colors (ARGB8888)
enum PPU_COLORS : u32 {
    PPU_WHITE = 0xFFFFFFFF,
    PPU_LIGHT_GRAY = 0xFFAAAAAA,
    PPU_DARK_GRAY = 0xFF555555,
    PPU_BLACK = 0xFF000000,
};
static const u8 PALETTE_SIZE = 4;

enum PPUMode {
    HBlank, VBlank, OAM, DRAW
};

class PPU {
public:
    PPU(InterruptController* interrupt_controller);
    void reset();
    void step(u8 cycles);

    u8 read(u16 addr) const;
    void write(u16 addr, u8 val);

    const std::vector<std::vector<u32>>& get_framebuffer() const;

    PPUMode get_mode();
    u8 get_ly();

private:
    InterruptController* interrupt_controller;

    std::vector<std::vector<u32>> framebuffer;
    std::vector<u8> vram;
    std::vector<u8> oam;
    std::vector<u32> palette;

    PPUMode mode;
    int cycle_counter;

    void render_scanline();

    void render_background();
    void render_window();
    void render_sprites();

    void update_stat_register();
    void check_lyc_interrupt();

    void clear_framebuffer();

    u8 lcdc;
    u8 stat;
    u8 scy;
    u8 scx;
    u8 ly;
    u8 lyc;
    u8 bgp;
    u8 obp0;
    u8 obp1;
    u8 wy;
    u8 wx;

    std::vector<u32> line;
};

