#pragma once

#include "../utils/types.hpp"



// ROM Regions
static const u16 STATIC_ROM_START = 0x0000;
static const u16 STATIC_ROM_END = 0x3FFF;
static const u16 STATIC_ROM_SIZE = 0x4000;

static const u16 SWITCHABLE_ROM_START = 0x4000;
static const u16 SWITCHABLE_ROM_END = 0x7FFF;
static const u16 SWITCHABLE_ROM_SIZE = 0x4000;

// VRAM
static const u16 VRAM_START = 0x8000;
static const u16 VRAM_END = 0x9FFF;
static const u16 VRAM_SIZE = 0x2000;

// External (Switchable) RAM
static const u16 SWITCHABLE_RAM_START = 0xA000;
static const u16 SWITCHABLE_RAM_END = 0xBFFF;
static const u16 SWITCHABLE_RAM_SIZE = 0x2000;

// Internal RAM
static const u16 INTERNAL_RAM_START = 0xC000;
static const u16 INTERNAL_RAM_END = 0xDFFF;
static const u16 INTERNAL_RAM_SIZE = 0x2000;

// Echo RAM
static const u16 ECHO_RAM_START = 0xE000;
static const u16 ECHO_RAM_END = 0xFDFF;
static const u16 ECHO_RAM_SIZE = 0x1DFF;

// Sprite Attribute Table (OAM)
static const u16 OAM_START = 0xFE00;
static const u16 OAM_END = 0xFE9F;
static const u16 OAM_SIZE = 0x00A0;

// I/O Registers
static const u16 IO_START = 0xFF00;
static const u16 IO_END = 0xFF4B;
static const u16 IO_SIZE = 0x004C;

// Timer Address Range
static const u16 TIMER_REGS_START = 0xFF04;
static const u16 TIMER_REGS_END = 0xFF07;

// PPU Registers Address Range
static const u16 PPU_REGS_START = 0xFF40;
static const u16 PPU_REGS_END = 0xFF4B;

// High RAM
static const u16 HIGH_RAM_START = 0xFF80;
static const u16 HIGH_RAM_END = 0xFFFE;
static const u16 HIGH_RAM_SIZE = 0x007F;

// Interrupt Register
static const u16 INTERRUPT_REGISTER_ADDR = 0xFFFF;

// MBC1 Control Ranges
static const u16 RAM_ENABLE_START = 0x0000;
static const u16 RAM_ENABLE_END = 0x1FFF;
static const u16 ROM_BANK_SELECT_START = 0x2000;
static const u16 ROM_BANK_SELECT_END = 0x3FFF;
static const u16 RAM_BANK_SELECT_START = 0x4000;
static const u16 RAM_BANK_SELECT_END = 0x5FFF;
static const u16 BANKING_MODE_START = 0x6000;
static const u16 BANKING_MODE_END = 0x7FFF;

// ROM Header Offsets
static const u16 HEADER_ROM_SIZE_ADDR = 0x0148;
static const u16 HEADER_RAM_SIZE_ADDR = 0x0149;

// Defaults
static const u8 DEFAULT_READ_RETURN = 0xFF;

// MBC1 Masks
static const u8 MBC1_ROM_BANKS_MASK = 0x1F;
static const u8 MBC1_RAM_BANKS_MASK = 0x03;
static const u8 MBC1_RAM_ENABLE_MASK = 0x0F;
static const u8 MBC1_RAM_ENABLE_ENABLED = 0x0A;

// LCD Dimensions
static const u8 LCD_WIDTH = 160;
static const u8 LCD_HEIGHT = 144;

// Tile Data Address Range
static const u16 TILE_DATA_START = 0x8000;
static const u16 TILE_DATA_END = 0x97FF;
static const u16 TILE_DATA_SIZE = 0x1800;

