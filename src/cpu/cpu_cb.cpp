#include "cpu.hpp"
#include "../memory/mmu.hpp"

void CPU::cb_ins_handler() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            fetch_next_instruction(cb_handlers_);
            return;
        default:
            throw std::runtime_error("Invalid state in cb_ins_handler");
    }
}

// ============================================================================
// CB prefix instructions
// ============================================================================

// Rotate, shift, and bit operation instructions
void CPU::op_rlca() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 value = getA();
            bool c_bit = value >> 7;
            value = (value << 1) | c_bit;
            setA(value);
            setFlagZ(0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rlca");
    }
}

void CPU::op_rrca() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 value = getA();
            bool c_bit = value & 0x01;
            value = (value >> 1) | (c_bit << 7);
            setA(value);
            setFlagZ(0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rrca");
    }
}
void CPU::op_rla() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 value = getA();
            bool c_bit = value >> 7;
            value = (value << 1) | getFlagC();
            setA(value);
            setFlagZ(0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rla");
    }
}

void CPU::op_rra() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 value = getA();
            bool c_bit = value & 0x01;
            value = (value >> 1) | (getFlagC() << 7);
            setA(value);
            setFlagZ(0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rra");
    }
}

void CPU::op_rlc_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value >> 7;
            value = (value << 1) | c_bit;
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rlc_r");
    }
}

void CPU::op_rlc_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value >> 7;
            value = (value << 1) | c_bit;
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_rlc_hl_ind");
    }
}

void CPU::op_rrc_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value & 0x01;
            value = (value >> 1) | (c_bit << 7);
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rrc_r");
    }
}

void CPU::op_rrc_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value & 0x01;
            value = (value >> 1) | (c_bit << 7);
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_rrc_hl_ind");
    }
}

void CPU::op_rl_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value >> 7;
            value = (value << 1) | getFlagC();
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rl_r");
    }
}

void CPU::op_rl_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value >> 7;
            value = (value << 1) | getFlagC();
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_rl_hl_ind");
    }
}

void CPU::op_rr_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value & 0x01;
            value = (value >> 1) | (getFlagC() << 7);
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_rr_r");
    }
}

void CPU::op_rr_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value & 0x01;
            value = (value >> 1) | (getFlagC() << 7);
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_rr_hl_ind");
    }
}

void CPU::op_sla_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value >> 7;
            value = value << 1;
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sla_r");
    }
}

void CPU::op_sla_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value >> 7;
            value = value << 1;
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_sla_hl_ind");
    }
}

void CPU::op_sra_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value & 0x01;
            value = (value >> 1) | (value & 0x80);
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sra_r");
    }
}

void CPU::op_sra_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value & 0x01;
            value = (value >> 1) | (value & 0x80);
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_sra_hl_ind");
    }
}

void CPU::op_swap_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            value = (value << 4) | (value >> 4);
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_swap_r");
    }
}

void CPU::op_swap_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            value = (value << 4) | (value >> 4);
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_swap_hl_ind");
    }
}

void CPU::op_srl_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(destination_register);
            bool c_bit = value & 0x01;
            value = value >> 1;
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_srl_r");
    }
}

void CPU::op_srl_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool c_bit = value & 0x01;
            value = value >> 1;
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(c_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_srl_hl_ind");
    }
}

void CPU::op_bit_b_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            setFlagZ(!test_bit(value, get_bits(current_opcode_, 5, 3)));
            setFlagN(0);
            setFlagH(1);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_bit_b_r");
    }
}
void CPU::op_bit_b_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setFlagZ(!test_bit(value, get_bits(current_opcode_, 5, 3)));
            setFlagN(0);
            setFlagH(1);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_bit_b_hl_ind");
    }
}

void CPU::op_res_b_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            value = clear_bit(value, get_bits(current_opcode_, 5, 3));
            write_register_8_bit(source_register, value);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_res_b_r");
    }
}

void CPU::op_res_b_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            value = clear_bit(value, get_bits(current_opcode_, 5, 3));
            write_memory_8(hl_, value);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_res_b_hl_ind");
    }
}

void CPU::op_set_b_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            value = set_bit(value, get_bits(current_opcode_, 5, 3), true);
            write_register_8_bit(source_register, value);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_set_b_r");
    }
}

void CPU::op_set_b_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            value = set_bit(value, get_bits(current_opcode_, 5, 3), true);
            write_memory_8(hl_, value);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_set_b_hl_ind");
    }
}
