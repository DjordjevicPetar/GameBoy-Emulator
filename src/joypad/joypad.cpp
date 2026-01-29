#include "joypad.hpp"
#include <iostream>

Joypad::Joypad(InterruptController* interrupt_controller)
    : interrupt_controller(interrupt_controller)
    , p1(0xCF)
    , button_state{}
{}

void Joypad::press(Button b) {
    
    update_button_state(b, true);
}

void Joypad::release(Button b) {
    update_button_state(b, false);
}

void Joypad::update_button_state(Button b, bool pressed) {
    bool prev = button_state[b];
    button_state[b] = pressed;

    if (!prev && pressed) {
        std::cout << "Pressed";
        interrupt_controller->request_interrupt(INTERRUPT_HANDLER_JOYPAD_ADDRESS);
    } else if (prev && !pressed) {
        std::cout << "Released";
    }
}

u8 Joypad::read(u16 addr) {
    u8 ret = p1;

    bool select_dpad = !(p1 & 0x10);
    bool select_buttons = !(p1 & 0x20);

    if (select_dpad) {
        if (button_state[Right]) ret &= ~0x01;
        if (button_state[Left]) ret &= ~0x02;
        if (button_state[Up]) ret &= ~0x04;
        if (button_state[Down]) ret &= ~0x08;
    }

    if (select_buttons) {
        if (button_state[A]) ret &= ~0x01;
        if (button_state[B]) ret &= ~0x02;
        if (button_state[Select]) ret &= ~0x04;
        if (button_state[Start]) ret &= ~0x08;
    }

    return ret;
}

void Joypad::write(u16 addr, u8 val) {
    p1 = (p1 & 0x0F) | (val & 0x30);
}