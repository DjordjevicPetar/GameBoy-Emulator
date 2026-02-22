#include "cpu.hpp"
#include "../memory/mmu.hpp"

// ============================================================================
// Control flow instructions
// ============================================================================

void CPU::op_jp_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            pc_ = memory_temp_;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_jp_imm");
    }
}

void CPU::op_jp_hl() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            pc_ = hl_;
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_jp_hl");
    }
}

void CPU::op_jp_cc_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
            if (read_condition_argument()) {
                state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            } else {
                state_machine_ = State_Machine::NO_OP;
            }
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u16 address = memory_temp_;
            pc_ = address;
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_jp_cc_imm");
    }
}

void CPU::op_jr_e() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            if (value >> 7) {
                value = ~value + 1;
                pc_ = pc_ - value;
            } else {
                pc_ = pc_ + value;
            }
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_jr_e");
    }
}

void CPU::op_jr_cc_e() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            if (read_condition_argument()) {
                state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            } else {
                memory_temp_ = pc_;
                state_machine_ = State_Machine::NO_OP;
            }
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            if (value >> 7) {
                value = ~value + 1;
                memory_temp_ = pc_ - value;
            } else {
                memory_temp_ = pc_ + value;
            }
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            pc_ = memory_temp_;
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_jr_cc_e");
    }
}

void CPU::op_call_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
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
            pc_ = memory_temp_;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_call_imm");
    }
}

void CPU::op_call_cc_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
            if (read_condition_argument()) {
                state_machine_ = State_Machine::DECREMENT_SP;
            } else {
                state_machine_ = State_Machine::NO_OP;
            }
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
            pc_ = memory_temp_;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_call_cc_imm");
    }
}


void CPU::op_ret() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(sp_);
            sp_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(sp_, true);
            sp_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            pc_ = memory_temp_;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ret");
    }
}

void CPU::op_ret_cc() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            if (read_condition_argument()) {
                state_machine_ = State_Machine::READ_MEMORY_LOW_BYTE;
            } else {
                state_machine_ = State_Machine::NO_OP;
            }
            return;
        case State_Machine::READ_MEMORY_LOW_BYTE:
            read_memory_8(sp_);
            sp_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(sp_, true);
            sp_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            pc_ = memory_temp_;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ret_cc");
    }
}

void CPU::op_reti() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(sp_);
            sp_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(sp_, true);
            sp_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            pc_ = memory_temp_;
            ime_ = true;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_reti");
    }
}

void CPU::op_rst_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
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
            pc_ = get_bits(current_opcode_, 5, 3) << 3;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_rst_imm");
    }
}

// ============================================================================
// Miscellaneous instructions
// ============================================================================

void CPU::op_halt() {
    // TODO(cycle-accuracy): HALT bug with multi-byte instructions — exact behavior
    // differs when the duplicated byte is part of a multi-byte instruction (can
    // corrupt execution). Low priority; very few games trigger this.

    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            if (!ime_ && interrupt_controller_->has_pending_interrupt()) {
                halt_bug_triggered_ = true;
            } else {
                halted_ = true;
            }
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_halt");
    }
}

void CPU::op_stop() {
    // TODO: Joypad wake-up not yet implemented (see joypad.cpp TODO).
    // On CGB, STOP also triggers a speed switch if KEY1 register is prepared.
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY;
            pc_++;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY:
            stopped_ = true;
            mmu_->write_memory_8(TIMER_REGISTER_DIV, 0x00);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_stop");
    }
}

void CPU::op_di() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            ime_ = false;
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_di");
    }
}

void CPU::op_ei() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            ei_pending_ = true;
            ei_was_pending_ = false;
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ei");
    }
}

void CPU::op_nop() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_nop");
    }
}
