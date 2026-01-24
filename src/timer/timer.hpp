#pragma once

#include "utils/types.hpp"


#include "utils/constants.hpp"

// Forward declaration
class InterruptController;

enum TIMER_REGISTER_ADDRESSES {
    TIMER_REGISTER_DIV = 0xFF04,
    TIMER_REGISTER_TIMA = 0xFF05,
    TIMER_REGISTER_TMA = 0xFF06,
    TIMER_REGISTER_TAC = 0xFF07,
};

class Timer {
public:
    explicit Timer(InterruptController* interrupt_controller);
    
    void update_timer(u32 cycles);
    void write_timer(u16 address, u8 value);
    u8 read_timer(u16 address) const;

private:
    bool has_enough_cycles_passed_tima() const;
    bool has_enough_cycles_passed_div() const;
    void update_tima();
    void update_div();

    InterruptController* interrupt_controller_;
    
    u8 div_register_ = 0;
    u8 tima_register_ = 0;
    u8 tma_register_ = 0;
    u8 tac_register_ = 0;

    u32 cycles_since_last_update_tima_ = 0;
    u32 cycles_since_last_update_div_ = 0;
};

