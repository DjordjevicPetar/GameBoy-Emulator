#include "cpu.hpp"
#include "../interrupt/interrupt_controller.hpp"
#include "../utils/logger.hpp"
#include "../memory/mmu.hpp"
#include "../timer/timer.hpp"
#include <iostream>
#include <stdexcept>

// ============================================================================
// CPU Implementation
// ============================================================================

CPU::CPU(MMU* mmu, InterruptController* interrupt_controller) 
    : mmu_(mmu)
    , interrupt_controller_(interrupt_controller) {
    // Initialize registers (DMG boot state)
    setA(0x01);
    setB(0x00);
    setC(0x13);
    setD(0x00);
    setE(0xD8);
    setH(0x01);
    setL(0x4D);
    pc_ = 0x0100;
    sp_ = 0xFFFE;
    
    // Initialize flags
    setFlagZ(true);
    setFlagN(false);
    setFlagH(true);
    setFlagC(true);
    
    // Initialize instruction handlers
    registerInstructions();
    registerCbInstructions();
}

void CPU::log_state() {
    // TODO: These reads bypass the state machine and may cause side effects
    // on memory-mapped I/O. Consider logging only register state.
    u8 mem0 = mmu_->read_memory_8(pc_);
    u8 mem1 = mmu_->read_memory_8(pc_ + 1);
    u8 mem2 = mmu_->read_memory_8(pc_ + 2);
    u8 mem3 = mmu_->read_memory_8(pc_ + 3);
    Logger::log((af_ >> 8) & 0xFF, af_ & 0xFF,
                (bc_ >> 8) & 0xFF, bc_ & 0xFF,
                (de_ >> 8) & 0xFF, de_ & 0xFF,
                (hl_ >> 8) & 0xFF, hl_ & 0xFF,
                sp_, pc_, mem0, mem1, mem2, mem3);
}

void CPU::process_cycle() {
    if (Logger::isEnabled() && !halted_) {
        log_state();
    }
    
    bool ei_was_pending = ei_pending_;
    ei_pending_ = false;

    if (process_interrupts_) {
        process_interrupts();
    } else if (!halted_ && state_machine_ != State_Machine::FETCH_NEXT_INSTRUCTION) {
        (this->*current_instruction_handler_)();
    }

    if (state_machine_ == State_Machine::FETCH_NEXT_INSTRUCTION) {
        detect_interrupts();

        if (!halted_ && !process_interrupts_) {
            fetch_next_instruction(op_handlers_);
            state_machine_ = State_Machine::INITIAL_PHASE;
        }
    }
    
    if (ei_was_pending) {
        ime_ = true;
    }
}

void CPU::detect_interrupts() {
    bool has_pending = interrupt_controller_->has_pending_interrupt();
    bool was_halted = halted_;

    if (halted_ && has_pending) {
        halted_ = false;
    }

    if (!ime_ || !has_pending) {
        return;
    }

    process_interrupts_ = true;
    // When waking from HALT, the wake-up cycle itself serves as M1
    // of the 5-cycle dispatch, so skip the PROCESS_INTERRUPTS NOP state.
    state_machine_ = was_halted ? State_Machine::DECREMENT_SP
                                : State_Machine::PROCESS_INTERRUPTS;
}

void CPU::process_interrupts() {
    switch (state_machine_) {
        case State_Machine::PROCESS_INTERRUPTS:
            state_machine_ = State_Machine::DECREMENT_SP;
            return;
        case State_Machine::DECREMENT_SP:
            sp_--;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY_HIGH_BYTE:
            write_memory_8(sp_, pc_ >> 8);
            sp_--;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY_LOW_BYTE;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY_LOW_BYTE:
            write_memory_8(sp_, pc_ & 0xFF);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            pc_ = interrupt_controller_->get_address_of_highest_priority_interrupt();
            ime_ = false;
            process_interrupts_ = false;
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in process_interrupts");
    }
}

void CPU::read_memory_8(u16 address, bool high_byte) {
    u8 value = mmu_->read_memory_8(address);
    if (high_byte) {
        memory_temp_ = (static_cast<u16>(value) << 8) | (memory_temp_ & 0xFF);
    } else {
        memory_temp_ = (memory_temp_ & 0xFF00) | value;
    }
}

void CPU::write_memory_8(u16 address, u8 value) {
    mmu_->write_memory_8(address, value);
}

u8 CPU::read_condition_argument() const {
    u8 cond = (current_opcode_ >> 3) & 0x03;
    switch (cond) {
        case 0: return !getFlagZ();
        case 1: return getFlagZ();
        case 2: return !getFlagC();
        case 3: return getFlagC();
        default: throw std::runtime_error("Invalid condition");
    }
}

// ============================================================================
// Register Access by Number
// ============================================================================

u8 CPU::read_register_8_bit(u8 reg_num) const {
    switch (reg_num) {
        case 0: return getB();
        case 1: return getC();
        case 2: return getD();
        case 3: return getE();
        case 4: return getH();
        case 5: return getL();
        case 6: return mmu_->read_memory_8(hl_);
        case 7: return getA();
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_8_bit(u8 reg_num, u8 value) {
    switch (reg_num) {
        case 0: setB(value); break;
        case 1: setC(value); break;
        case 2: setD(value); break;
        case 3: setE(value); break;
        case 4: setH(value); break;
        case 5: setL(value); break;
        case 6: mmu_->write_memory_8(hl_, value); break;
        case 7: setA(value); break;
        default: throw std::runtime_error("Invalid register number");
    }
}

u16 CPU::read_register_16_bit(u8 reg_num) const {
    switch (reg_num) {
        case 0: return bc_;
        case 1: return de_;
        case 2: return hl_;
        case 3: return sp_;
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit(u8 reg_num, u16 value) {
    switch (reg_num) {
        case 0: bc_ = value; break;
        case 1: de_ = value; break;
        case 2: hl_ = value; break;
        case 3: sp_ = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

u16 CPU::read_register_16_bit_stack(u8 reg_num) const {
    switch (reg_num) {
        case 0: return bc_;
        case 1: return de_;
        case 2: return hl_;
        case 3: return af_;
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit_stack(u8 reg_num, u16 value) {
    switch (reg_num) {
        case 0: bc_ = value; break;
        case 1: de_ = value; break;
        case 2: hl_ = value; break;
        case 3: af_ = value & 0xFFF0; break;  // Lower 4 bits of F are always 0
        default: throw std::runtime_error("Invalid register number");
    }
}

