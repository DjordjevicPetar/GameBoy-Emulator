#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <cstdint>

// CPU Constants
static const uint16_t PROGRAM_COUNTER_START = 0x0100;

// CPU Clock Speeds (Hz)
// Cycle Relationships
// 1 M-cycle = 4 T-cycles
static const uint32_t DMG_CLOCK_SPEED = 4194304;  // 4.194304 MHz

#endif