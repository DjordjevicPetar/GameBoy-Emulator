#include "timer.hpp"
#include "../interrupt/interrupt_controller.hpp"


Timer::Timer(InterruptController* interrupt_controller) 
    : interrupt_controller_(interrupt_controller)
    , div_register_(0xAB)
    , tima_register_(0x00)
    , tma_register_(0x00)
    , tac_register_(0xF8)
    , cycles_since_last_update_tima_(0)
    , cycles_since_last_update_div_(0) {}

bool Timer::has_enough_cycles_passed_tima() const {
    if ((tac_register_ & 0x04) == 0x00) {
        return false;
    }
    return cycles_since_last_update_tima_ >= (DMG_CLOCK_SPEED / TAC_FREQUENCIES[tac_register_ & 0x03]);
}

bool Timer::has_enough_cycles_passed_div() const {
    return cycles_since_last_update_div_ >= (DMG_CLOCK_SPEED / DIV_FREQUENCY);
}

void Timer::update_timer(u32 cycles) {
    cycles_since_last_update_tima_ += cycles;
    cycles_since_last_update_div_ += cycles;
    update_tima();
    update_div();
}

void Timer::update_tima() {
    while (has_enough_cycles_passed_tima()) {
        tima_register_++;
        if (tima_register_ == 0x00) {
            tima_register_ = tma_register_;
            interrupt_controller_->request_interrupt(INTERRUPT_TIMER_BIT);
        }
        cycles_since_last_update_tima_ -= (DMG_CLOCK_SPEED / TAC_FREQUENCIES[tac_register_ & 0x03]);
    }
}

void Timer::update_div() {
    while (has_enough_cycles_passed_div()) {
        div_register_++;
        cycles_since_last_update_div_ -= (DMG_CLOCK_SPEED / DIV_FREQUENCY);
    }
}

void Timer::write_timer(u16 address, u8 value) {
    switch (address) {
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_DIV:
            div_register_ = 0x00;
            break;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TIMA:
            tima_register_ = value;
            break;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TMA:
            tma_register_ = value;
            break;
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_TAC:
            tac_register_ = value;
            break;
    }
}

u8 Timer::read_timer(u16 address) const {
    switch (address) {
        case TIMER_REGISTER_ADDRESSES::TIMER_REGISTER_DIV:
            return div_register_;
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