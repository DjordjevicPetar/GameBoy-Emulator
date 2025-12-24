#include "../inc/interrupt_controller.hpp"
#include <stdexcept>


InterruptController::InterruptController() 
    : ie_(0x00)
    , if_(0xE1) {}

void InterruptController::request_interrupt(uint8_t interrupt_bit) {
    if_ |= (1 << interrupt_bit);
}

void InterruptController::clear_interrupt(uint8_t interrupt_bit) {
    if_ &= ~(1 << interrupt_bit);
}

void InterruptController::write_interrupt(uint16_t address, uint8_t value) {
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

uint8_t InterruptController::read_interrupt(uint16_t address) const {
    switch (address) {
        case IE_REGISTER_LOCATION:
            return ie_;
        case IF_REGISTER_LOCATION:
            return if_;
        default:
            throw std::runtime_error("Invalid interrupt register address");
    }
}

uint16_t InterruptController::get_address_of_highest_priority_interrupt() {
    // Check interrupts in priority order: VBlank > LCD STAT > Timer > Serial > Joypad
    
    if ((if_ & (1 << INTERRUPT_VBLANK_BIT)) && (ie_ & (1 << INTERRUPT_VBLANK_BIT))) {
        clear_interrupt(INTERRUPT_VBLANK_BIT);
        return INTERRUPT_HANDLER_VBLANK_ADDRESS;
    }
    
    if ((if_ & (1 << INTERRUPT_LCD_STAT_BIT)) && (ie_ & (1 << INTERRUPT_LCD_STAT_BIT))) {
        clear_interrupt(INTERRUPT_LCD_STAT_BIT);
        return INTERRUPT_HANDLER_LCD_STAT_ADDRESS;
    }
    
    if ((if_ & (1 << INTERRUPT_TIMER_BIT)) && (ie_ & (1 << INTERRUPT_TIMER_BIT))) {
        clear_interrupt(INTERRUPT_TIMER_BIT);
        return INTERRUPT_HANDLER_TIMER_ADDRESS;
    }
    
    if ((if_ & (1 << INTERRUPT_SERIAL_BIT)) && (ie_ & (1 << INTERRUPT_SERIAL_BIT))) {
        clear_interrupt(INTERRUPT_SERIAL_BIT);
        return INTERRUPT_HANDLER_SERIAL_ADDRESS;
    }
    
    if ((if_ & (1 << INTERRUPT_JOYPAD_BIT)) && (ie_ & (1 << INTERRUPT_JOYPAD_BIT))) {
        clear_interrupt(INTERRUPT_JOYPAD_BIT);
        return INTERRUPT_HANDLER_JOYPAD_ADDRESS;
    }
    
    return INTERRUPT_HANDLER_NONE_ADDRESS;
}