#ifndef TIMER_H
#define TIMER_H

#include "../inc/constants.hpp"

// Forward declaration
class InterruptController;

class Timer {

private:
    InterruptController* interrupt_controller;
    
    uint8_t div_register;
    uint8_t tima_register;
    uint8_t tma_register;
    uint8_t tac_register;

    uint32_t cycles_since_last_update_tima;
    uint32_t cycles_since_last_update_div;

    bool has_enough_cycles_passed_tima();
    bool has_enough_cycles_passed_div();

    void update_tima();
    void update_div();

public:
    Timer(InterruptController* interrupt_controller);
    void update_timer(uint32_t cycles);
    void write_timer(uint16_t address, uint8_t value);
    uint8_t read_timer(uint16_t address);
};
#endif