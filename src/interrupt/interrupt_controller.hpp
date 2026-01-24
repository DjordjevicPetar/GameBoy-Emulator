#pragma once

#include "utils/types.hpp"

#include "utils/constants.hpp"

// Interrupt Handler Addresses (ISR vectors)
enum INTERRUPT_HANDLER_ADDRESSES {
    INTERRUPT_HANDLER_VBLANK_ADDRESS = 0x0040,
    INTERRUPT_HANDLER_LCD_STAT_ADDRESS = 0x0048,
    INTERRUPT_HANDLER_TIMER_ADDRESS = 0x0050,
    INTERRUPT_HANDLER_SERIAL_ADDRESS = 0x0058,
    INTERRUPT_HANDLER_JOYPAD_ADDRESS = 0x0060,
    INTERRUPT_HANDLER_NONE_ADDRESS = 0xFFFF,
};

class InterruptController {
public:
    InterruptController();
    
    void request_interrupt(u8 interrupt_bit);
    void clear_interrupt(u8 interrupt_bit);
    void write_interrupt(u16 address, u8 value);
    u8 read_interrupt(u16 address) const;
    bool has_pending_interrupt() const;
    u16 get_address_of_highest_priority_interrupt();

private:
    u8 ie_ = 0;  // Interrupt Enable register
    u8 if_ = 0;  // Interrupt Flag register
};
