#pragma once

#include <cstdint>

// Unsigned integers
using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

// Signed integers
using s8  = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

// Verify sizes at compile time
static_assert(sizeof(u8)  == 1, "u8 must be 1 byte");
static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");
static_assert(sizeof(s8)  == 1, "s8 must be 1 byte");
static_assert(sizeof(s16) == 2, "s16 must be 2 bytes");
static_assert(sizeof(s32) == 4, "s32 must be 4 bytes");
static_assert(sizeof(s64) == 8, "s64 must be 8 bytes");

// ============================================================================
// Bit Manipulation Helpers
// ============================================================================

// Get single bit (u8 version)
[[nodiscard]] constexpr bool get_bit(u8 value, unsigned bit) {
    return (value >> bit) & 1;
}

// Get single bit (u16 version)
[[nodiscard]] constexpr bool get_bit(u16 value, unsigned bit) {
    return (value >> bit) & 1;
}

// Set single bit (u8 version)
[[nodiscard]] constexpr u8 set_bit(u8 value, unsigned bit, bool new_val) {
    if (new_val) {
        return value | (1U << bit);
    }
    return value & ~(1U << bit);
}

// Set single bit (u16 version)
[[nodiscard]] constexpr u16 set_bit(u16 value, unsigned bit, bool new_val) {
    if (new_val) {
        return value | (1U << bit);
    }
    return value & ~(1U << bit);
}

// Clear single bit (convenience function)
[[nodiscard]] constexpr u8 clear_bit(u8 value, unsigned bit) {
    return value & ~(1U << bit);
}

// Test if bit is set
[[nodiscard]] constexpr bool test_bit(u8 value, unsigned bit) {
    return (value & (1U << bit)) != 0;
}

// Extract bits [hi:lo] from value
[[nodiscard]] constexpr u16 get_bits(u16 value, unsigned hi, unsigned lo) {
    u16 mask = ((1U << (hi - lo + 1)) - 1) << lo;
    return (value & mask) >> lo;
}

// Set bits [hi:lo] in value
[[nodiscard]] constexpr u16 set_bits(u16 value, unsigned hi, unsigned lo, u16 new_bits) {
    u16 mask = ((1U << (hi - lo + 1)) - 1) << lo;
    return (value & ~mask) | ((new_bits << lo) & mask);
}

// ============================================================================
// Sign Extension Helpers
// ============================================================================

// Sign-extend 8-bit to 16-bit
[[nodiscard]] constexpr s16 sign_extend8(u8 value) {
    return static_cast<s16>(static_cast<s8>(value));
}
