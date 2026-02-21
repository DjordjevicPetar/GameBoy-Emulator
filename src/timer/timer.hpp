#pragma once

#include "../utils/types.hpp"


#include "../utils/constants.hpp"

// Forward declaration
class InterruptController;

enum TIMER_REGISTER_ADDRESSES {
    TIMER_REGISTER_DIV = 0xFF04,
    TIMER_REGISTER_TIMA = 0xFF05,
    TIMER_REGISTER_TMA = 0xFF06,
    TIMER_REGISTER_TAC = 0xFF07,
};

constexpr u8 TAC_BIT_LOOKUP[4] = {9, 3, 5, 7};

class Timer {
public:
    explicit Timer(InterruptController* interrupt_controller);
    
    void write_timer(u16 address, u8 value);
    u8 read_timer(u16 address) const;

    void process_cycle();

private:
    InterruptController* interrupt_controller_;

    u16 sys_counter_ = 0;
    
    u8 tima_register_ = 0;
    u8 tma_register_ = 0;
    u8 tac_register_ = 0;

    bool overflow_delay = false;

    void update_tima(u16 new_counter, u16 old_counter);
};

