#ifndef TIMER_HPP_
#define TIMER_HPP_

#include "constants.hpp"
#include <cstdint>

// Forward declaration
class InterruptController;

class Timer {
public:
    explicit Timer(InterruptController* interrupt_controller);
    
    void update_timer(uint32_t cycles);
    void write_timer(uint16_t address, uint8_t value);
    uint8_t read_timer(uint16_t address) const;

private:
    bool has_enough_cycles_passed_tima() const;
    bool has_enough_cycles_passed_div() const;
    void update_tima();
    void update_div();

    InterruptController* interrupt_controller_;
    
    uint8_t div_register_ = 0;
    uint8_t tima_register_ = 0;
    uint8_t tma_register_ = 0;
    uint8_t tac_register_ = 0;

    uint32_t cycles_since_last_update_tima_ = 0;
    uint32_t cycles_since_last_update_div_ = 0;
};

#endif