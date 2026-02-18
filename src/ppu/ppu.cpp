#include "ppu.hpp"
#include "../memory/mmu.hpp"

PPU::PPU(InterruptController* interrupt_controller) :
    interrupt_controller(interrupt_controller),
    framebuffer(LCD_HEIGHT, std::vector<u32>(LCD_WIDTH, 0)),
    vram(VRAM_SIZE),
    oam(OAM_SIZE),
    palette(PALETTE_SIZE),
    line(LCD_WIDTH, 0),
    bg_color_id_line(LCD_WIDTH, 0)
{

    lcdc = 0x91;
    stat = 0x85;
    scy = 0x00;
    scx = 0x00;
    ly = 0x00;
    lyc = 0x00;
    bgp = 0xFC;
    obp0 = 0xFF;
    obp1 = 0xFF;
    wy = 0x00;
    wx = 0x00;

    mode = OAM;
    cycle_counter = 0;

    palette[0] = PPU_WHITE;
    palette[1] = PPU_LIGHT_GRAY;
    palette[2] = PPU_DARK_GRAY;
    palette[3] = PPU_BLACK;

    window_line_counter = 0;
    window_was_rendered = false;
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

const std::vector<std::vector<u32>>& PPU::get_framebuffer() const {
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
    u32 white = 0xFFFFFFFF;

    for (int y = 0; y < LCD_HEIGHT; y++) {
        for (int x = 0; x < LCD_WIDTH; x++) {
            framebuffer[y][x] = white;
        }
    }
}

u8 PPU::read(u16 addr) const {
    if (addr >= VRAM_START && addr <= VRAM_END) {
        if (mode == DRAW) return DEFAULT_READ_RETURN;
        return vram[addr - VRAM_START];
    }
    else if (addr >= OAM_START && addr <= OAM_END) {
        if (mode == OAM || mode == DRAW) return DEFAULT_READ_RETURN;
        return oam[addr - OAM_START];
    }
    switch (addr) {
        case PPU_REGISTER_LCDC: return lcdc;
        case PPU_REGISTER_STAT: return (stat & STAT_READ_WRITE_MASK) | (lyc == ly ? STAT_LY_COMPARE_TRUE : STAT_LY_COMPARE_FALSE) | (mode & STAT_MODE_MASK);
        case PPU_REGISTER_SCY: return scy;
        case PPU_REGISTER_SCX: return scx;
        case PPU_REGISTER_LY: return ly;
        case PPU_REGISTER_LYC: return lyc;
        case PPU_REGISTER_BGP: return bgp;
        case PPU_REGISTER_OBP0: return obp0;
        case PPU_REGISTER_OBP1: return obp1;
        case PPU_REGISTER_WY: return wy;
        case PPU_REGISTER_WX: return wx;

        default: return DEFAULT_READ_RETURN;
    }
}

void PPU::write(u16 addr, u8 val) {
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
        case PPU_REGISTER_LCDC: {
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
                window_line_counter = 0;
                window_was_rendered = false;
            }
            return;
        }
        case PPU_REGISTER_STAT: {
            stat = (stat & STAT_READ_ONLY_MASK) | (val & STAT_READ_WRITE_MASK);
            return;
        }
        case PPU_REGISTER_SCY: {
            scy = val;
            return;
        }
        case PPU_REGISTER_SCX: {
            scx = val;
            return;
        }
        case PPU_REGISTER_LY: {
            // 'ly' is read only (it automatically updates in step())
            return;
        }
        case PPU_REGISTER_LYC: {
            lyc = val;
            if (lyc == ly && (stat & STAT_LYC_INTERRUPT_ENABLE_MASK)) {
                interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            }
            update_stat_register();
            return;
        }
        case PPU_REGISTER_BGP: {
            bgp = val;
            return;
        }
        case PPU_REGISTER_OBP0: {
            obp0 = val;
            return;
        }
        case PPU_REGISTER_OBP1: {
            obp1 = val;
            return;
        }
        case PPU_REGISTER_WY: {
            wy = val;
            return;
        }
        case PPU_REGISTER_WX: {
            wx = val;
            return;
        }
        default: {
            return;
        }
    }
}

void PPU::step(u8 cycles) {
    if (!(lcdc & LCDC_PPU_ENABLE_MASK)) return;

    cycle_counter += cycles;

    // TODO(cycle-accuracy): Mode 3 (DRAW) duration is hardcoded at 172 T-cycles, but on
    // real hardware it varies from 172 to ~289 T-cycles depending on:
    //  - Number of sprites on the current scanline (each sprite adds ~6-11 cycles)
    //  - SCX % 8 (initial tile alignment adds 0-7 cycles of penalty)
    //  - Window trigger mid-scanline (restarts the pixel FIFO)
    // HBlank duration adjusts to keep total line at 456 T-cycles.
    // This affects HBlank timing which games use for mid-frame VRAM updates.

    // TODO(cycle-accuracy): STAT interrupt should use an "IRQ line" model. On real HW,
    // the LCD STAT interrupt is edge-triggered: it only fires when the combined STAT
    // condition (mode match OR LYC==LY) transitions from LOW to HIGH. The current
    // implementation fires on every mode change unconditionally, which can produce
    // duplicate STAT interrupts and break games that rely on exact interrupt timing
    // (e.g.,Ings/GBVideoPlayer, Road Rash). Fix: track a `stat_irq_line` bool,
    // compute new line state each step, and only request interrupt on rising edge.

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
                window_line_counter = 0;
                window_was_rendered = false;
                check_lyc_interrupt();
                mode = OAM;
                update_stat_register();
                if (stat & STAT_OAM_INTERRUPT_ENABLE_MASK) interrupt_controller->request_interrupt(INTERRUPT_LCD_STAT_BIT);
            }
        }
    }
}

void PPU::render_scanline() {
    // TODO(cycle-accuracy): The entire scanline is rendered at once when entering HBlank.
    // On real hardware, pixels are pushed out one at a time via the pixel FIFO during
    // mode 3 (DRAW). This means mid-scanline register changes (SCX, SCY, BGP, palette
    // writes) take effect immediately on the current pixel. Games like Prehistorik Man
    // and demos like "oh!" rely on this for raster effects. Implementing a pixel FIFO
    // would fix these and also give us accurate mode 3 timing for free.
    
    for (int i = 0; i < LCD_WIDTH; i++) {
        bg_color_id_line[i] = 0;
        line[i] = palette[0];
    }

    render_background();
    render_window();
    render_sprites();

    if (window_was_rendered) {
        window_line_counter++;
        window_was_rendered = false;
    }

    framebuffer[ly] = line;
}

void PPU::render_background() {
    if (!(lcdc & LCDC_BG_AND_WIN_ENABLE_MASK)) return; // Background disabled
    u16 tile_map_base = (lcdc & LCDC_BG_TILE_MAP_AREA_MASK) ? BACKGROUND_TILE_MAP1_START : BACKGROUND_TILE_MAP0_START;

    u8 bg_y = (scy + ly) % 256;
    u8 tile_row = bg_y / 8;
    u8 tile_row_pixel = bg_y % 8;

    u16 tile_addr;

    for (u8 x = 0; x < LCD_WIDTH; x++) {
        u8 bg_x = (scx + x) % 256;
        u8 tile_col = bg_x / 8;
        u8 tile_col_pixel = bg_x % 8;

        u8 tile_id = vram[tile_map_base - VRAM_START + tile_row * 32 + tile_col];

        if (lcdc & LCDC_BG_AND_WIN_TILE_DATA_AREA_MASK) {
            u16 tile_data_base = 0x8000;
            tile_addr = tile_data_base + tile_id * 16;
        }
        else {
            u16 tile_data_base = 0x9000;
            tile_addr = tile_data_base + (s8)tile_id * 16;
        }

        u8 byte0 = vram[tile_addr - VRAM_START + tile_row_pixel * 2];
        u8 byte1 = vram[tile_addr - VRAM_START + tile_row_pixel * 2 + 1];

        u8 bit_index = 7 - tile_col_pixel;
        u8 bit0 = (byte0 >> bit_index) & 1;
        u8 bit1 = (byte1 >> bit_index) & 1;

        u8 color_id = (bit1 & 1) << 1 | (bit0 & 1);

        bg_color_id_line[x] = color_id;
        
        u8 shade = (bgp >> (color_id * 2)) & 0x03;
        line[x] = palette[shade];
    }
}

void PPU::render_window() {
    if (!(lcdc & LCDC_BG_AND_WIN_ENABLE_MASK)) return; // Background and Window disabled
    if (!(lcdc & LCDC_WIN_ENABLE_MASK)) return; // Window disabled
    if (ly < wy) return; // Didn't get to the starting point yet
    if (wx > 166) return;

    u16 tile_map_base = (lcdc & LCDC_WIN_TILE_MAP_AREA_MASK) ? BACKGROUND_TILE_MAP1_START : BACKGROUND_TILE_MAP0_START;
    
    u8 win_y = window_line_counter;
    u8 tile_row = win_y / 8;
    u8 tile_row_pixel = win_y % 8;

    u16 tile_addr;

    for (int x = 0; x < LCD_WIDTH; x++) {
        s16 win_x = x - (wx - 7);

        if (win_x < 0) continue;
        if (win_x >= 160) break;

        u8 tile_col = win_x / 8;
        u8 tile_col_pixel = win_x % 8;

        u8 tile_id = vram[tile_map_base - VRAM_START + tile_row * 32 + tile_col];

        if (lcdc & LCDC_BG_AND_WIN_TILE_DATA_AREA_MASK) {
            u16 tile_data_base = 0x8000;
            tile_addr = tile_data_base + tile_id * 16;
        }
        else {
            u16 tile_data_base = 0x9000;
            tile_addr = tile_data_base + (s8)tile_id * 16;
        }

        u8 byte0 = vram[tile_addr - VRAM_START + tile_row_pixel * 2];
        u8 byte1 = vram[tile_addr - VRAM_START + tile_row_pixel * 2 + 1];

        u8 bit_index = 7 - tile_col_pixel;
        u8 bit0 = (byte0 >> bit_index) & 1;
        u8 bit1 = (byte1 >> bit_index) & 1;

        u8 color_id = (bit1 & 1) << 1 | (bit0 & 1);

        bg_color_id_line[x] = color_id;
        
        u8 shade = (bgp >> (color_id * 2)) & 0x03;
        line[x] = palette[shade];
    }
    window_was_rendered = true;
}

void PPU::render_sprites() {
    if (!(lcdc & LCDC_OBJ_ENABLE_MASK)) return;

    // TODO(cycle-accuracy): Sprite evaluation (OAM scan) should happen during mode 2
    // (OAM search, 80 T-cycles), not during rendering. The OAM scan checks 2 sprites
    // per M-cycle (40 sprites in 80 cycles). The 10-sprite-per-line limit and the
    // order they're found in affects mode 3 timing and priority. Currently we scan
    // OAM during render_scanline() which is called at the end of mode 3.

    bool sprite_size = lcdc & LCDC_OBJ_SIZE_MASK;

    struct Sprite {
        u8 y, x;
        u8 tile;
        u8 flags;
        u8 oam_index;
    };

    std::vector<Sprite> sprites(10);
    int count = 0;

    for (int i = 0; i < 40; i++) {
        u8 y = oam[i * 4 + 0];
        u8 x = oam[i * 4 + 1];
        u8 tile = oam[i * 4 + 2];
        u8 flags = oam[i * 4 + 3];

        int sprite_y = y - 16;

        if (ly < sprite_y || ly >= sprite_y + (sprite_size ? 16 : 8)) continue;

        if (count < 10) {
            sprites[count++] = {y, x, tile, flags, (u8)i};
        }
    }

    if (count == 0) return;

    std::sort(sprites.begin(), sprites.begin() + count, [](const Sprite& a, const Sprite& b) {
        if (a.x != b.x) return a.x > b.x;
        return a.oam_index > b.oam_index;
    });

    for (int i = 0; i < count; i++) {
        u8 y = sprites[i].y;
        u8 x = sprites[i].x;
        u8 tile = sprites[i].tile;
        u8 flags = sprites[i].flags;

        int sprite_y = y - 16;
        int sprite_x = x - 8;

        bool flip_x = flags & 0x20;
        bool flip_y = flags & 0x40;
        bool behind_bg = flags & 0x80;
        u8 palette_id = flags & 0x10 ? obp1 : obp0;

        int local_y = ly - sprite_y;
        if (flip_y) {
            local_y = (sprite_size ? 15 : 7) - local_y;
        }

        if (sprite_size) {
            tile &= 0xFE; // bit0 not used
        }

        u16 tile_addr = 0x8000 + tile * 16;
        tile_addr += local_y * 2;

        u8 byte0 = vram[tile_addr - VRAM_START];
        u8 byte1 = vram[tile_addr - VRAM_START + 1];

        for (int px = 0; px < 8; px++) {
            int screen_x = sprite_x + px;
            if (screen_x < 0 || screen_x >= 160) continue;

            int bit_select = flip_x ? px : (7 - px);

            u8 bit0 = (byte0 >> bit_select) & 1;
            u8 bit1 = (byte1 >> bit_select) & 1;
            u8 color_id = (bit1 << 1) | bit0;

            if (color_id == 0) continue;

            bool bg_win_priority = (lcdc & 0x01);

            if (bg_win_priority && behind_bg) {
                if (bg_color_id_line[screen_x] != 0) continue;
            }

            u8 shade = (palette_id >> (color_id * 2)) & 0x03;

            line[screen_x] = palette[shade];
        }
    }
}

PPUMode PPU::get_mode() {
    return mode;
}

u8 PPU::get_ly() {
    return ly;
}

void PPU::write_dma(u8 val) {
    // TODO(cycle-accuracy): OAM DMA takes 160 M-cycles (640 T-cycles) and should
    // transfer one byte per M-cycle. During DMA, the CPU can ONLY access HRAM
    // (0xFF80-0xFFFE) - all other reads return 0xFF. Currently the entire 160-byte
    // transfer happens instantly in zero cycles. This breaks games that rely on
    // the DMA timing (most games use a small HRAM routine to wait for DMA completion).
    // Fix: set dma_active=true, track dma_counter, transfer 1 byte per M-cycle in
    // step(), and have MMU return 0xFF for non-HRAM reads while dma_active.
    dma_source = val * 0x100;
    dma_counter = 0;
    dma_active = true;

    for (int i = 0; i < 160; ++i) {
        oam[i] = mmu->read_memory_8(dma_source + i);
    }

    dma_active = false;
}