#include "../inc/ppu.hpp"

PPU::PPU() :
    framebuffer(LCD_HEIGHT, std::vector<uint32_t>(LCD_WIDTH, 0)),
    vram(VRAM_SIZE),
    oam(OAM_SIZE)
{
    mode = OAM;
    ly = 0;
    cycle_counter = 0;
}

void PPU::reset() {
    mode = OAM;
    ly = 0;
    cycle_counter = 0;

    for (auto& row: framebuffer) {
        row.assign(LCD_WIDTH, 0);
    }

    vram.assign(VRAM_SIZE, 0);
    oam.assign(OAM_SIZE, 0);
}

uint8_t PPU::read(uint16_t addr) const {
    if (addr >= VRAM_START && addr <= VRAM_END) {
        return vram[addr - VRAM_START];
    }
    else if (addr >= OAM_START && addr <= OAM_END) {
        return oam[addr - OAM_START];
    }
    switch (addr) {
        case LCDC_ADDR: return lcdc;
        case STAT_ADDR: return stat; // TODO
        case SCY_ADDR: return scy;
        case SCX_ADDR: return scx;
        case LY_ADDR: return ly;
        case LYC_ADDR: return lyc;
        case BGP_ADDR: return bgp;
        case OBP0_ADDR: return obp0;
        case OBP1_ADDR: return obp1;
        case WY_ADDR: return wy;
        case WX_ADDR: return wx;

        default: return DEFAULT_READ_RETURN;
    }
}

void PPU::write(uint16_t addr, uint8_t val) {
    if (addr >= VRAM_START && addr <= VRAM_END) {
        vram[addr - VRAM_START] = val;
        return;
    }
    else if (addr >= OAM_START && addr <= OAM_END) {
        oam[addr - OAM_START] = val;
        return;
    }
    // TODO: PPU Register writes
}

void PPU::step(uint8_t cycles) {
    // TODO
}