#include "timer.hpp"
#include "../interrupt/interrupt_controller.hpp"

// TODO(cycle-accuracy/critical): The entire timer model is wrong for cycle-accurate emulation.
// On real hardware, the timer is driven by a 16-bit internal counter ("sys_counter") that
// increments every T-cycle. DIV (0xFF04) exposes the UPPER 8 bits of this counter.
// TIMA increments on the FALLING EDGE of a specific bit of sys_counter, selected by TAC:
//   TAC & 0x03:  0 -> bit 9 (every 1024 T-cycles = 4096 Hz)
//                1 -> bit 3 (every 16 T-cycles = 262144 Hz)
//                2 -> bit 5 (every 64 T-cycles = 65536 Hz)
//                3 -> bit 7 (every 256 T-cycles = 16384 Hz)
// The falling-edge detector output is: (selected_bit AND tac_enable). TIMA ticks when
// this signal goes from 1 to 0. This means:
//   1. Writing ANY value to DIV resets sys_counter to 0. If the selected bit was 1,
//      this forces a falling edge -> TIMA increments (the "DIV write glitch").
//   2. Changing TAC frequency can similarly cause a spurious TIMA tick if the old
//      selected bit was 1 and the new one is 0.
//   3. Disabling the timer via TAC bit 2 while the selected bit is 1 also ticks TIMA.
// Additionally, when TIMA overflows (0xFF -> 0x00):
//   - There's a 4 T-cycle delay before TMA is loaded into TIMA and the interrupt fires.
//   - During this delay, writing to TIMA cancels the overflow. Writing to TMA during
//     this delay updates the value that will be loaded.
// None of these behaviors are implemented. Replace with:
//   u16 sys_counter_;  // incremented by T-cycles each update
//   bool prev_edge_;   // previous falling-edge detector state
//   u8 overflow_countdown_;  // 4-cycle delay for TMA reload
// This is REQUIRED to pass blargg's timer tests (instr_timing will still need
// per-M-cycle stepping from the main loop).

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
        // TODO(cycle-accuracy): TIMA overflow should NOT immediately reload TMA.
        // There is a 4 T-cycle delay: during the delay, TIMA reads as 0x00.
        // If TIMA is written during the delay, the overflow is cancelled.
        // If TMA is written during the delay, the new TMA value is used.
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
            // TODO(cycle-accuracy): Writing to DIV should reset the entire 16-bit
            // internal counter to 0, which can cause a TIMA increment if the
            // selected bit was 1 (falling edge). Currently we also don't reset
            // the cycle accumulators, which causes drift.
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