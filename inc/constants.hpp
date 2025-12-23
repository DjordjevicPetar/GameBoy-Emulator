#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <cstdint>

// CPU Constants
static const uint16_t PROGRAM_COUNTER_START = 0x0100;

// CPU Clock Speeds (Hz)
// Cycle Relationships
// 1 M-cycle = 4 T-cycles
static const uint32_t DMG_CLOCK_SPEED = 4194304;  // 4.194304 MHz

// Timer register locations
static const uint16_t DIV_REGISTER_LOCATION = 0xFF04; // Divider register, incremented by 1 every 16384 Hz
static const uint16_t TIMA_REGISTER_LOCATION = 0xFF05; // Value in this register is incremented by 1 at the frequency specified by the TAC register
static const uint16_t TMA_REGISTER_LOCATION = 0xFF06; // Value in this register is loaded into TIMA when it overflows
static const uint16_t TAC_REGISTER_LOCATION = 0xFF07; // Timer control register

// Interrupts bit locations
static const uint8_t INTERRUPT_VBLANK_BIT = 0;
static const uint8_t INTERRUPT_LCD_STAT_BIT = 1;
static const uint8_t INTERRUPT_TIMER_BIT = 2;
static const uint8_t INTERRUPT_SERIAL_BIT = 3;
static const uint8_t INTERRUPT_JOYPAD_BIT = 4;

// Interrupt register locations
static const uint16_t IE_REGISTER_LOCATION = 0xFFFF;
static const uint16_t IF_REGISTER_LOCATION = 0xFF0F;

// Interrupt handler addresses
static const uint16_t INTERRUPT_HANDLER_VBLANK_ADDRESS = 0x0040;
static const uint16_t INTERRUPT_HANDLER_LCD_STAT_ADDRESS = 0x0048;
static const uint16_t INTERRUPT_HANDLER_TIMER_ADDRESS = 0x0050;
static const uint16_t INTERRUPT_HANDLER_SERIAL_ADDRESS = 0x0058;
static const uint16_t INTERRUPT_HANDLER_JOYPAD_ADDRESS = 0x0060;
static const uint16_t INTERRUPT_HANDLER_NONE_ADDRESS = 0xFFFF;

// Timer Frequencies
static const uint32_t DIV_FREQUENCY = 16384;
static const uint32_t TAC_FREQUENCIES[4] = {4096, 262144, 65536, 16384};

#endif