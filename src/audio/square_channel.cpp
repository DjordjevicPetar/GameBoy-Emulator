#include "square_channel.hpp"

SquareChannel::SquareChannel(bool has_sweep) :
    has_sweep(has_sweep)
{
    reset();
}

void SquareChannel::reset() {
    nrx0 = 0;
    nrx1 = 0;
    nrx2 = 0;
    nrx3 = 0;
    nrx4 = 0;

    enabled = false;
    dac_enabled = false;

    frequency = 0;
    timer = 0;

    duty_pos = 0;

    sweep_timer = 0;
    sweep_enabled = false;
    shadow_register = 0;

    sweep_pace = 0;
    sweep_negate = false;
    sweep_step = 0;

    length_timer = 0;
    length_enabled = false;

    env_timer = 0;
    env_pace = 0;
    env_increase = 0;
    env_volume = 0;
    current_volume = 0;
}

u8 SquareChannel::read_nrx0() {
    return nrx0 | 0x80;
}

u8 SquareChannel::read_nrx1() {
    return nrx1 | 0x3F;
}

u8 SquareChannel::read_nrx2() {
    return nrx2;
}

u8 SquareChannel::read_nrx3() {
    return 0xFF;
}

u8 SquareChannel::read_nrx4() {
    return nrx4 | 0xBF; 
}

void SquareChannel::write_nrx0(u8 val) {
    if (!has_sweep) return;

    nrx0 = val;

    sweep_pace = (val >> 4) & 0x07;
    sweep_negate = (val >> 3) & 0x01;
    sweep_step = val & 0x07;
    
    sweep_enabled = sweep_pace != 0 || sweep_step != 0;
}

void SquareChannel::write_nrx1(u8 val) {
    nrx1 = val;

    wave_duty = (val >> 6) & 0x03;
    length_timer = 64 - (val & 0x3F); // Since length_timer is counting up, but my implemetation is counting down
}

void SquareChannel::write_nrx2(u8 val) {
    nrx2 = val;

    env_volume = (val >> 4) & 0x0F;
    env_increase = (val >> 3) & 0x01;
    env_pace = val & 0x07;

    dac_enabled = (val & 0xF8) != 0;

    if (!dac_enabled) {
        enabled = false;
    }
}

void SquareChannel::write_nrx3(u8 val) {
    nrx3 = val;

    frequency = (frequency & 0x0700) | val;
}

void SquareChannel::write_nrx4(u8 val) {
    nrx4 = val;

    frequency = (frequency & 0x00FF) | ((val & 0x07) << 8);
    length_enabled = (val & 0x40) != 0;
    
    if (val & 0x80) {
        trigger();
    }
}

void SquareChannel::trigger() {
    enabled = dac_enabled;

    if (length_timer == 0) {
        length_timer = 64;
    }

    timer = (2048 - frequency) * 4;

    env_timer = (env_pace == 0) ? 8 : env_pace;
    current_volume = env_volume;

    if (has_sweep) {
        shadow_register = frequency;

        sweep_timer = (sweep_pace == 0) ? 8 : sweep_pace;

        sweep_enabled = sweep_pace != 0 || sweep_step != 0;

        if (sweep_step > 0) {
            u16 new_freq = calculate_sweep_frequency();

            if (new_freq > 2047) {
                enabled = false;
            }
        }
    }
}

u16 SquareChannel::calculate_sweep_frequency() {
    if (sweep_negate) {
        return shadow_register - (shadow_register >> sweep_step);
    } else {
        return shadow_register + (shadow_register >> sweep_step);
    }
}

void SquareChannel::step(u8 cycles) {
    if (!enabled) return;

    timer -= cycles;
    while (timer <= 0) {
        timer += (2048 - frequency) * 4;
        duty_pos = (duty_pos + 1) % 8;
    }
}

void SquareChannel::clock_sweep() {
    if (!has_sweep) return;

    if (sweep_timer > 0) {
        sweep_timer--;
    }

    if (sweep_timer == 0) {
        sweep_timer = (sweep_pace == 0) ? 8 : sweep_pace;

        if (sweep_enabled && sweep_pace != 0) {
            u16 new_freq = calculate_sweep_frequency();

            if (new_freq > 2047) {
                enabled = false;
                return;
            }

            if (sweep_step != 0) {
                shadow_register = new_freq;
                frequency = new_freq;

                new_freq = calculate_sweep_frequency();

                if (new_freq > 2047) {
                    enabled = false;
                    return;
                }
            }
        } 
    }
    
    
}

void SquareChannel::clock_envelope() {
    if (env_pace == 0) return;

    if (env_timer > 0) {
        env_timer--;
    }

    if (env_timer == 0) {
        env_timer = (env_pace == 0) ? 8 : env_pace;

        if (env_increase) {
            if (current_volume < 15) current_volume++;
        } else {
            if (current_volume > 0) current_volume--;
        }
    }
}

void SquareChannel::clock_sound_length() {
    if (!length_enabled) return;

    if (length_timer > 0) {
        length_timer--;
    }

    if (length_timer == 0) {
        enabled = false;
        return;
    }
}

u8 SquareChannel::output() {
    if (!enabled || !dac_enabled) return 0;

    u8 bit = duty_table[wave_duty][duty_pos];

    return ~bit ? current_volume : 0;
}