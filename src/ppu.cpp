#include "../inc/ppu.hpp"

PPU::PPU(InterruptController* interrupt_controller) :
    interrupt_controller(interrupt_controller),
    framebuffer(LCD_HEIGHT, std::vector<uint32_t>(LCD_WIDTH, 0)),
    vram(VRAM_SIZE),
    oam(OAM_SIZE),
    palette(PALETTE_SIZE)
{
    mode = OAM;
    ly = 0;
    cycle_counter = 0;

    palette[0] = PPU_WHITE;
    palette[1] = PPU_LIGHT_GRAY;
    palette[2] = PPU_DARK_GRAY;
    palette[3] = PPU_BLACK;
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

void PPU::update_stat_register() {
    stat = (stat & STAT_READ_WRITE_MASK) | (lyc == ly ? STAT_LY_COMPARE_TRUE : STAT_LY_COMPARE_FALSE) | (mode & STAT_MODE_MASK);
}

void PPU::check_lyc_interrupt() {
    update_stat_register();
    if (lyc == ly && (stat & STAT_LYC_INTERRUPT_ENABLE_MASK)) {
        interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
    }
}

void PPU::clear_framebuffer() {
    uint32_t white = 0xFFFFFFFF;

    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            framebuffer[y][x] = white;
        }
    }
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
        case STAT_ADDR: return (stat & STAT_READ_WRITE_MASK) | (lyc == ly ? STAT_LY_COMPARE_TRUE : STAT_LY_COMPARE_FALSE) | (mode & STAT_MODE_MASK);
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
        case LCDC_ADDR: {
            bool lcd_enable_prev = lcdc & LCDC_PPU_ENABLE_MASK;
            bool lcd_enable_new = val & LCDC_PPU_ENABLE_MASK;

            lcdc = val;
            if (lcd_enable_prev && !lcd_enable_new) {
                mode = HBlank;
                ly = 0;
                cycle_counter = 0;
                clear_framebuffer();
            }

            else if (!lcd_enable_prev && lcd_enable_new) {
                mode = OAM;
                ly = 0;
                cycle_counter = 0;
            }
            return;
        }
        case STAT_ADDR: {
            stat = (stat & STAT_READ_ONLY_MASK) | (val & STAT_READ_WRITE_MASK);
            return;
        }
        case SCY_ADDR: {
            scy = val;
            return;
        }
        case SCX_ADDR: {
            scx = val;
            return;
        }
        case LY_ADDR: {
            // 'ly' is read only (it automatically updates in step())
            return;
        }
        case LYC_ADDR: {
            lyc = val;
            if (lyc == ly && (stat & STAT_LYC_INTERRUPT_ENABLE_MASK)) {
                interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            }
            update_stat_register();
            return;
        }
        case BGP_ADDR: {
            bgp = val;
            return;
        }
        case OBP0_ADDR: {
            obp0 = val;
            return;
        }
        case OBP1_ADDR: {
            obp1 = val;
            return;
        }
        case WY_ADDR: {
            wy = val;
            return;
        }
        case WX_ADDR: {
            wx = val;
            return;
        }
        default: {
            return;
        }
    }
}

void PPU::step(uint8_t cycles) {
    if (!(lcdc & LCDC_PPU_ENABLE_MASK)) return;

    cycle_counter += cycles;

    if (mode == OAM) {
        if (cycle_counter >= PPU_OAM_CYCLES) {
            mode = DRAW;
            update_stat_register();
        }
    }
    else if (mode == DRAW) {
        if (cycle_counter >= PPU_OAM_CYCLES + PPU_DRAW_CYCLES) {
            mode = HBlank;
            update_stat_register();
            if (stat & STAT_HBLANK_INTERRUPT_ENABLE_MASK) interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            render_scanline();
        }
    }
    else if (mode == HBlank) {
        if (cycle_counter >= PPU_FULL_LINE_CYCLES) {
            cycle_counter -= PPU_FULL_LINE_CYCLES;
            ly++;
            check_lyc_interrupt();

            if (ly == PPU_VBLANK_FIRST_LINE) {
                mode = VBlank;
                update_stat_register();
                interrupt_controller->request_interrupt(INTERRUPT_VBLANK_BIT);
                if (stat & STAT_VBLANK_INTERRUPT_ENABLE_MASK) interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            }
            else {
                mode = OAM;
                update_stat_register();
                if (stat & STAT_OAM_INTERRUPT_ENABLE_MASK) interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            }
        }
    }
    else if (mode == VBlank) {
        if (cycle_counter >= PPU_FULL_LINE_CYCLES) {
            cycle_counter -= PPU_FULL_LINE_CYCLES;
            ly++;
            check_lyc_interrupt();

            if (ly > PPU_VBLANK_LAST_LINE) {
                ly = 0;
                check_lyc_interrupt();
                mode = OAM;
                update_stat_register();
                if (stat & STAT_OAM_INTERRUPT_ENABLE_MASK) interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            }
        }
    }
}

void PPU::render_scanline() {
    std::cout << "Render Scanline called.\n";

    render_background();
    render_window();
    render_sprites();

    framebuffer[ly] = line;
}

void PPU::render_background() {
    if (!(lcdc & LCDC_BG_AND_WIN_ENABLE_MASK)) return; // Background disabled
    uint16_t tile_map_base = (lcdc & LCDC_BG_TILE_MAP_AREA_MASK) ? BACKGROUND_TILE_MAP1_START : BACKGROUND_TILE_MAP0_START;

    uint8_t bg_y = (scy + ly) % 256;
    uint8_t tile_row = bg_y / 8;
    uint8_t tile_row_pixel = bg_y % 8;

    uint16_t tile_addr;

    for (uint8_t x = 0; x < LCD_WIDTH; x++) {
        uint8_t bg_x = (scx + x) % 256;
        uint8_t tile_col = bg_x / 8;
        uint8_t tile_col_pixel = bg_x % 8;

        uint8_t tile_id = vram[tile_map_base - VRAM_START + tile_row * 32 + tile_col];

        if (lcdc & LCDC_BG_AND_WIN_TILE_DATA_AREA_MASK) {
            uint16_t tile_data_base = 0x8000;
            tile_addr = tile_data_base + tile_id * 16;
        }
        else {
            uint16_t tile_data_base = 0x9000;
            tile_addr = tile_data_base + (int8_t)tile_id * 16;
        }

        uint8_t byte0 = vram[tile_addr - VRAM_START + tile_row_pixel * 2];
        uint8_t byte1 = vram[tile_addr - VRAM_START + tile_row_pixel * 2 + 1];

        uint8_t bit_index = 7 - tile_col_pixel;
        uint8_t bit0 = (byte0 >> bit_index) & 1;
        uint8_t bit1 = (byte1 >> bit_index) & 1;

        uint8_t color_id = (bit1 & 1) << 1 | (bit0 & 1);

        line[x] = palette[color_id];
    }
}

void PPU::render_window() {
    if (!(lcdc & LCDC_BG_AND_WIN_ENABLE_MASK)) return; // Background and Window disabled
    if (!(lcdc & LCDC_WIN_ENABLE_MASK)) return; // Window disabled
    if (ly < wy) return; // Didn't get to the starting point yet

    uint16_t tile_map_base = (lcdc & LCDC_WIN_TILE_MAP_AREA_MASK) ? BACKGROUND_TILE_MAP1_START : BACKGROUND_TILE_MAP0_START;
    
    uint8_t win_y = ly - wy;
    uint8_t tile_row = win_y / 8;
    uint8_t tile_row_pixel = win_y % 8;

    uint16_t tile_addr;

    for (int x = 0; x < LCD_WIDTH; x++) {
        int16_t win_x = x - (wx - 7);

        if (win_x < 0) continue;
        if (win_x >= 160) break;

        uint8_t tile_col = win_x / 8;
        uint8_t tile_col_pixel = win_x % 8;

        uint8_t tile_id = vram[tile_map_base - VRAM_START + tile_row * 32 + tile_col];

        if (lcdc & LCDC_BG_AND_WIN_TILE_DATA_AREA_MASK) {
            uint16_t tile_data_base = 0x8000;
            tile_addr = tile_data_base + tile_id * 16;
        }
        else {
            uint16_t tile_data_base = 0x9000;
            tile_addr = tile_data_base + (int8_t)tile_id * 16;
        }

        uint8_t byte0 = vram[tile_addr - VRAM_START + tile_row_pixel * 2];
        uint8_t byte1 = vram[tile_addr - VRAM_START + tile_row_pixel * 2 + 1];

        uint8_t bit_index = 7 - tile_col_pixel;
        uint8_t bit0 = (byte0 >> bit_index) & 1;
        uint8_t bit1 = (byte1 >> bit_index) & 1;

        uint8_t color_id = (bit1 & 1) << 1 | (bit0 & 1);

        line[x] = palette[color_id];
    }
}

void PPU::render_sprites() {
    if (!(lcdc & LCDC_OBJ_ENABLE_MASK)) return;

    bool sprite_size = lcdc & LCDC_OBJ_SIZE_MASK;

    struct Sprite {
        uint8_t y, x;
        uint8_t tile;
        uint8_t flags;
    };

    Sprite sprites[10];
    int count = 0;

    for (int i = 0; i < 40; i++) {
        uint8_t y = oam[i * 4 + 0];
        uint8_t x = oam[i * 4 + 1];
        uint8_t tile = oam[i * 4 + 2];
        uint8_t flags = oam[i * 4 + 3];

        int sprite_y = y - 16;

        if (ly < sprite_y || ly >= sprite_y + (sprite_size ? 16 : 8)) continue;

        if (count < 10) {
            sprites[count++] = {y, x, tile, flags};
        }
    }

    if (count == 0) return;

    for (int i = 0; i < count; i++) {
        uint8_t y = sprites[i].y;
        uint8_t x = sprites[i].x;
        uint8_t tile = sprites[i].tile;
        uint8_t flags = sprites[i].flags;

        int sprite_y = y - 16;
        int sprite_x = x - 8;

        bool flip_x = flags & 0x20;
        bool flip_y = flags & 0x40;
        bool behind_bg = flags & 0x80;
        uint8_t palette_id = flags & 0x10 ? obp1 : obp0;

        int local_y = ly - sprite_y;
        if (flip_y) {
            local_y = (sprite_size ? 15 : 7) - local_y;
        }

        if (sprite_size) {
            tile &= 0xFE; // bit0 not used
        }

        uint16_t tile_addr = 0x8000 + tile * 16;
        tile_addr += local_y * 2;

        uint8_t byte0 = vram[tile_addr - VRAM_START];
        uint8_t byte1 = vram[tile_addr - VRAM_START + 1];

        for (int px = 0; px < 8; px++) {
            int screen_x = sprite_x + px;
            if (screen_x < 0 || screen_x >= 160) continue;

            int bit_select = flip_x ? px : (7 - px);

            uint8_t bit0 = (byte0 >> bit_select) & 1;
            uint8_t bit1 = (byte1 >> bit_select) & 1;
            uint8_t color_id = (bit1 << 1) | bit0;

            if (color_id == 0) continue;

            if (behind_bg) {
                uint32_t bg_col = line[screen_x];
                if (bg_col != palette[0]) continue;
            }

            uint8_t shade = (palette_id >> (color_id * 2)) & 0x03;

            line[screen_x] = palette[shade];
        }
    }
}

PPUMode PPU::get_mode() {
    return mode;
}

uint8_t PPU::get_ly() {
    return ly;
}