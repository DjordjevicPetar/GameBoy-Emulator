#include "../inc/timer.hpp"

Timer::Timer() {
    this->div_register = 0xAB;
    this->tima_register = 0x00;
    this->tma_register = 0x00;
    this->tac_register = 0xF8;
    this->cycles_since_last_update_tima = 0;
    this->cycles_since_last_update_div = 0;
}

bool Timer::has_enough_cycles_passed_tima() {
    if ((tac_register & 0x04) == 0x00) {
        return false;
    }
    return cycles_since_last_update_tima >= (DMG_CLOCK_SPEED / TAC_FREQUENCIES[tac_register & 0x03]);
}

bool Timer::has_enough_cycles_passed_div() {
    return cycles_since_last_update_div >= (DMG_CLOCK_SPEED / DIV_FREQUENCY);
}

void Timer::update_timer(uint32_t cycles) {
    cycles_since_last_update_tima += cycles;
    cycles_since_last_update_div += cycles;
    update_tima();
    update_div();
}

void Timer::update_tima() {
    while (!has_enough_cycles_passed_tima()) {
        tima_register++;
        if (tima_register == 0x00) {
            tima_register = tma_register;
            // TODO Request interrpupt
        }
        cycles_since_last_update_tima -= (DMG_CLOCK_SPEED / TAC_FREQUENCIES[tac_register & 0x03]);
    }
}

void Timer::update_div() {
    while (!has_enough_cycles_passed_div()) {
        div_register++;
        cycles_since_last_update_div -= (DMG_CLOCK_SPEED / DIV_FREQUENCY);
    }
}

void Timer::write_timer(uint16_t address, uint8_t value) {
    switch (address) {
        case DIV_REGISTER_LOCATION:
            this->div_register = 0x00;
            break;
        case TIMA_REGISTER_LOCATION:
            this->tima_register = value;
            break;
        case TMA_REGISTER_LOCATION:
            this->tma_register = value;
            break;
        case TAC_REGISTER_LOCATION:
            this->tac_register = value;
            break;
    }
}

uint8_t Timer::read_timer(uint16_t address) {
    switch (address) {
        case DIV_REGISTER_LOCATION:
            return div_register;
        case TIMA_REGISTER_LOCATION:
            return tima_register;
        case TMA_REGISTER_LOCATION:
            return tma_register;
        case TAC_REGISTER_LOCATION:
            return tac_register;
        default:
            return 0x00;
    }
}