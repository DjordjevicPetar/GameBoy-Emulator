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

// Timer Frequencies
static const uint32_t DIV_FREQUENCY = 16384;
static const uint32_t TAC_FREQUENCIES[4] = {4096, 262144, 65536, 16384};

#endif