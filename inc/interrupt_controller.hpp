#ifndef INTERRUPT_CONTROLLER_HPP_
#define INTERRUPT_CONTROLLER_HPP_

#include <cstdint>
#include <stdexcept>
#include "../inc/constants.hpp"

class InterruptController {
    private:
        uint8_t IE; // Interrupt enable register
        uint8_t IF; // Interrupt flag register
    public:
        InterruptController();
        void request_interrupt(uint8_t interrupt_bit);
        void clear_interrupt(uint8_t interrupt_bit);
        void write_interrupt(uint16_t address, uint8_t value);
        uint8_t read_interrupt(uint16_t address) const;
        uint16_t get_address_of_highest_priority_interrupt();
};

#endif