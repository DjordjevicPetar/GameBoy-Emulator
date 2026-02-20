#include "timer.hpp"
#include "../interrupt/interrupt_controller.hpp"


Timer::Timer(InterruptController* interrupt_controller) 
    : interrupt_controller_(interrupt_controller)
    , sys_counter_(0xAB00)
    , tima_register_(0x00)
    , tma_register_(0x00)
    , tac_register_(0xF8) {}

void Timer::process_cycle() {
    u16 old_sys_counter = sys_counter_;
    sys_counter_ += 4;
    
    // Check if the selected bit has a falling edge
    if (overflow_delay) {
        overflow_delay = false;
        tima_register_ = tma_register_;
        interrupt_controller_->request_interrupt(INTERRUPT_TIMER_BIT);
    }
    update_tima(sys_counter_, old_sys_counter);
}

void Timer::update_tima(u16 sys_counter, u16 old_sys_counter) {
    u8 tac_bit = TAC_BIT_LOOKUP[tac_register_ & 0x03];
    if (!get_bit(sys_counter, tac_bit) && get_bit(old_sys_counter, tac_bit) && get_bit(tac_register_, 2)) {
        tima_register_++;
        if (tima_register_ == 0x00) overflow_delay = true;
    }
}

void Timer::write_timer(u16 address, u8 value) {
    switch (address) {
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_DIV:
            update_tima(0, sys_counter_);
            sys_counter_ = 0;
            break;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TIMA:
            tima_register_ = value;
            overflow_delay = false;
            break;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TMA:
            tma_register_ = value;
            break;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TAC: {
            bool old_signal = get_bit(sys_counter_, TAC_BIT_LOOKUP[tac_register_ & 0x03]) && (tac_register_ & 0x04);
            bool new_signal = get_bit(sys_counter_, TAC_BIT_LOOKUP[value & 0x03]) && (value & 0x04);
            if (old_signal && !new_signal) {
                tima_register_++;
                if (tima_register_ == 0x00) overflow_delay = true;
            }
            tac_register_ = value;
            break;
        }
    }
}

u8 Timer::read_timer(u16 address) const {
    switch (address) {
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_DIV:
            // Returns the upper 8 bits of the sys_counter
            return sys_counter_ >> 8;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TIMA:
            return tima_register_;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TMA:
            return tma_register_;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TAC:
            return tac_register_;
        default:
            return 0x00;
    }
}