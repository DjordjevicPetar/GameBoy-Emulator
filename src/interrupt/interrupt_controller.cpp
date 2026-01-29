#include "interrupt_controller.hpp"
#include "../utils/types.hpp"
#include <stdexcept>

InterruptController::InterruptController() 
    : ie_(0x00)
    , if_(0xE0) {}  // Upper 3 bits are always 1, lower 5 bits are interrupt flags

void InterruptController::request_interrupt(u8 interrupt_bit) {
    if_ = set_bit(if_, interrupt_bit, true);
}

void InterruptController::clear_interrupt(u8 interrupt_bit) {
    if_ = clear_bit(if_, interrupt_bit);
}

void InterruptController::write_interrupt(u16 address, u8 value) {
    switch (address) {
        case IE_REGISTER_LOCATION:
            ie_ = value;
            break;
        case IF_REGISTER_LOCATION:
            if_ = value;
            break;
        default:
            throw std::runtime_error("Invalid interrupt register address");
    }
}

u8 InterruptController::read_interrupt(u16 address) const {
    switch (address) {
        case IE_REGISTER_LOCATION:
            return ie_;
        case IF_REGISTER_LOCATION:
            return if_;
        default:
            throw std::runtime_error("Invalid interrupt register address");
    }
}

bool InterruptController::has_pending_interrupt() const {
    // Check if any enabled interrupt is pending
    return (if_ & ie_ & 0x1F) != 0;
}

u16 InterruptController::get_address_of_highest_priority_interrupt() {
    // Check interrupts in priority order: VBlank > LCD STAT > Timer > Serial > Joypad
    
    if (test_bit(if_, INTERRUPT_VBLANK_BIT) && test_bit(ie_, INTERRUPT_VBLANK_BIT)) {
        clear_interrupt(INTERRUPT_VBLANK_BIT);
        return INTERRUPT_HANDLER_VBLANK_ADDRESS;
    }
    
    if (test_bit(if_, INTERRUPT_LCD_STAT_BIT) && test_bit(ie_, INTERRUPT_LCD_STAT_BIT)) {
        clear_interrupt(INTERRUPT_LCD_STAT_BIT);
        return INTERRUPT_HANDLER_LCD_STAT_ADDRESS;
    }
    
    if (test_bit(if_, INTERRUPT_TIMER_BIT) && test_bit(ie_, INTERRUPT_TIMER_BIT)) {
        clear_interrupt(INTERRUPT_TIMER_BIT);
        return INTERRUPT_HANDLER_TIMER_ADDRESS;
    }
    
    if (test_bit(if_, INTERRUPT_SERIAL_BIT) && test_bit(ie_, INTERRUPT_SERIAL_BIT)) {
        clear_interrupt(INTERRUPT_SERIAL_BIT);
        return INTERRUPT_HANDLER_SERIAL_ADDRESS;
    }
    
    if (test_bit(if_, INTERRUPT_JOYPAD_BIT) && test_bit(ie_, INTERRUPT_JOYPAD_BIT)) {
        clear_interrupt(INTERRUPT_JOYPAD_BIT);
        return INTERRUPT_HANDLER_JOYPAD_ADDRESS;
    }
    
    return INTERRUPT_HANDLER_NONE_ADDRESS;
}
