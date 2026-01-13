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

const std::vector<std::vector<uint32_t>>& PPU::get_framebuffer() const {
    return framebuffer;
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
        case STAT_ADDR: return (stat & 0x78) | (lyc == ly ? 0x04 : 0x00) | (mode & 0x03);
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
        if (mode == DRAW) return;
        vram[addr - VRAM_START] = val;
        return;
    }
    else if (addr >= OAM_START && addr <= OAM_END) {
        if (mode == OAM || mode == DRAW) return;
        oam[addr - OAM_START] = val;
        return;
    }
    switch (addr) {
        case LCDC_ADDR:
            bool lcd_enable_prev = lcdc & 0x80;
            bool lcd_enable_new = val & 0x80;

            lcdc = val;
            if (lcd_enable_prev && !lcd_enable_new) {
                mode = HBlank;
                ly = 0;
                cycle_counter = 0;
            }

            else if (!lcd_enable_prev && lcd_enable_new) {
                mode = HBlank;
                ly = 0;
                cycle_counter = 0;
            }
            return;
        case STAT_ADDR:
            stat = (stat & 0x07) | (val & 0x78);
            return;
        case SCY_ADDR:
            scy = val;
            return;
        case SCX_ADDR:
            scx = val;
            return;
        case LY_ADDR:
            // 'ly' is read only (it automatically updates in step())
            return;
        case LYC_ADDR:
            lyc = val;
            return;
        case BGP_ADDR:
            bgp = val;
            return;
        case OBP0_ADDR:
            obp0 = val;
            return;
        case OBP1_ADDR:
            obp1 = val;
            return;
        case WY_ADDR:
            wy = val;
            return;
        case WX_ADDR:
            wx = val;
            return;
        default:
            return;
    }
}

void PPU::step(uint8_t cycles) {
    cycle_counter += cycles;

    if (mode == OAM) {
        if (cycle_counter >= PPU_OAM_CYCLES) {
            mode = DRAW;
        }
    }
    else if (mode == DRAW) {
        if (cycle_counter >= PPU_OAM_CYCLES + PPU_DRAW_CYCLES) {
            mode = HBlank;
            render_scanline();
        }
    }
    else if (mode == HBlank) {
        if (cycle_counter >= PPU_FULL_LINE_CYCLES) {
            cycle_counter -= PPU_FULL_LINE_CYCLES;
            ly++;

            if (ly == PPU_VBLANK_FIRST_LINE) {
                mode = VBlank;
            }
            else {
                mode = OAM;
            }
        }
    }
    else if (mode == VBlank) {
        if (cycle_counter >= PPU_FULL_LINE_CYCLES) {
            cycle_counter -= PPU_FULL_LINE_CYCLES;
            ly++;
            if (ly > PPU_VBLANK_LAST_LINE) {
                ly = 0;
                mode = OAM;
            }
        }
    }

    // stat bits update
    stat = (stat & 0x78) | (lyc == ly ? 0x04 : 0x00) | (mode & 0x03);
}