#pragma once

#include "../utils/types.hpp"

// CPU Constants
static const u16 PROGRAM_COUNTER_START = 0x0100;

// CPU Clock Speeds (Hz)
// Cycle Relationships
// 1 M-cycle = 4 T-cycles
static const u32 DMG_CLOCK_SPEED = 4194304;  // 4.194304 MHz

// Interrupts bit locations
static const u8 INTERRUPT_VBLANK_BIT = 0;
static const u8 INTERRUPT_LCD_STAT_BIT = 1;
static const u8 INTERRUPT_TIMER_BIT = 2;
static const u8 INTERRUPT_SERIAL_BIT = 3;
static const u8 INTERRUPT_JOYPAD_BIT = 4;

// Interrupt register locations
static const u16 IE_REGISTER_LOCATION = 0xFFFF;
static const u16 IF_REGISTER_LOCATION = 0xFF0F;
