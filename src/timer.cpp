#include "../inc/timer.hpp"
#include "../inc/interrupt_controller.hpp"


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

void Timer::update_timer(uint32_t cycles) {
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

void Timer::write_timer(uint16_t address, uint8_t value) {
    switch (address) {
        case DIV_REGISTER_LOCATION:
            div_register_ = 0x00;
            break;
        case TIMA_REGISTER_LOCATION:
            tima_register_ = value;
            break;
        case TMA_REGISTER_LOCATION:
            tma_register_ = value;
            break;
        case TAC_REGISTER_LOCATION:
            tac_register_ = value;
            break;
    }
}

uint8_t Timer::read_timer(uint16_t address) const {
    switch (address) {
        case DIV_REGISTER_LOCATION:
            return div_register_;
        case TIMA_REGISTER_LOCATION:
            return tima_register_;
        case TMA_REGISTER_LOCATION:
            return tma_register_;
        case TAC_REGISTER_LOCATION:
            return tac_register_;
        default:
            return 0x00;
    }
}