#ifndef _CONSTANTS_MMU_HPP_
#define _CONSTANTS_MMU_HPP_

// ROM Regions
static const uint16_t STATIC_ROM_START = 0x0000;
static const uint16_t STATIC_ROM_END = 0x3FFF;
static const uint16_t STATIC_ROM_SIZE = 0x4000;

static const uint16_t SWITCHABLE_ROM_START = 0x4000;
static const uint16_t SWITCHABLE_ROM_END = 0x7FFF;
static const uint16_t SWITCHABLE_ROM_SIZE = 0x4000;

// VRAM
static const uint16_t VRAM_START = 0x8000;
static const uint16_t VRAM_END = 0x9FFF;
static const uint16_t VRAM_SIZE = 0x2000;

// External (Switchable) RAM
static const uint16_t SWITCHABLE_RAM_START = 0xA000;
static const uint16_t SWITCHABLE_RAM_END = 0xBFFF;
static const uint16_t SWITCHABLE_RAM_SIZE = 0x2000;

// Internal RAM
static const uint16_t INTERNAL_RAM_START = 0xC000;
static const uint16_t INTERNAL_RAM_END = 0xDFFF;
static const uint16_t INTERNAL_RAM_SIZE = 0x2000;

// Sprite Attribute Table (OAM)
static const uint16_t OAM_START = 0xFE00;
static const uint16_t OAM_END = 0xFE9F;
static const uint16_t OAM_SIZE = 0x00A0;

// I/O Registers
static const uint16_t IO_START = 0xFF00;
static const uint16_t IO_END = 0xFF4B;
static const uint16_t IO_SIZE = 0x004C;

// High RAM
static const uint16_t HIGH_RAM_START = 0xFF80;
static const uint16_t HIGH_RAM_END = 0xFFFE;
static const uint16_t HIGH_RAM_SIZE = 0x007F;

// Interrupt Register
static const uint16_t INTERRUPT_REGISTER_ADDR = 0xFFFF;

// MBC1 Control Ranges
static const uint16_t RAM_ENABLE_START = 0x0000;
static const uint16_t RAM_ENABLE_END = 0x1FFF;
static const uint16_t ROM_BANK_SELECT_START = 0x2000;
static const uint16_t ROM_BANK_SELECT_END = 0x3FFF;
static const uint16_t RAM_BANK_SELECT_START = 0x4000;
static const uint16_t RAM_BANK_SELECT_END = 0x5FFF;
static const uint16_t BANKING_MODE_START = 0x6000;
static const uint16_t BANKING_MODE_END = 0x7FFF;

// ROM Header Offsets
static const uint16_t HEADER_ROM_SIZE_ADDR = 0x0148;
static const uint16_t HEADER_RAM_SIZE_ADDR = 0x0149;

// Defaults
static const uint8_t DEFAULT_READ_RETURN = 0xFF;

// MBC1 Masks
static const uint8_t MBC1_ROM_BANKS_MASK = 0x1F;
static const uint8_t MBC1_RAM_BANKS_MASK = 0x03;
static const uint8_t MBC1_RAM_ENABLE_MASK = 0x0F;
static const uint8_t MBC1_RAM_ENABLE_ENABLED = 0x0A;

// LCD Dimensions
static const uint8_t LCD_WIDTH = 160;
static const uint8_t LCD_HEIGHT = 144;

// PPU registers
static const uint16_t LCDC_ADDR = 0xFF40;
static const uint16_t STAT_ADDR = 0xFF41;
static const uint16_t SCY_ADDR = 0xFF42;
static const uint16_t SCX_ADDR = 0xFF43;
static const uint16_t LY_ADDR = 0xFF44;
static const uint16_t LYC_ADDR = 0xFF45;
static const uint16_t DMA_ADDR = 0xFF46;
static const uint16_t BGP_ADDR = 0xFF47;
static const uint16_t OBP0_ADDR = 0xFF48;
static const uint16_t OBP1_ADDR = 0xFF49;
static const uint16_t WY_ADDR = 0xFF4A;
static const uint16_t WX_ADDR = 0xFF4B;

#endif