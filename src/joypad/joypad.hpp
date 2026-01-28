#pragma once

#include "../utils/types.hpp"
#include "../interrupt/interrupt_controller.hpp"



class Joypad {
public:
    Joypad(InterruptController* interrupt_controller);

    enum Button {
        A, B, Select, Start,
        Right, Left, Up, Down
    };

    void press(Button b);
    void release(Button b);

    u8 read(u16 addr);
    void write(u16 addr, u8 val);
private:
    InterruptController* interrupt_controller;

    u8 p1;

    u8 select_mask;
    u8 buttons;
    u8 dpad;

    bool button_state[8];

    void update_button_state(Button b, bool pressed);
};