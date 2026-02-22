#include "cpu.hpp"
#include "../memory/mmu.hpp"

// 8-bit arithmetic and logical instructions
void CPU::op_add_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) > 0xFF;
            setA(getA() + value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_add_r");
    }
}

void CPU::op_add_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) > 0xFF;
            setA(getA() + value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_add_hl_ind");
    }
}

void CPU::op_add_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) > 0xFF;
            setA(getA() + value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_add_imm");
    }
}

void CPU::op_adc_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) + getFlagC() > 0xFF;
            setA(getA() + value + getFlagC());
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_adc_r");
    }
}

void CPU::op_adc_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) + getFlagC() > 0xFF;
            setA(getA() + value + getFlagC());
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_adc_hl_ind");
    }
}

void CPU::op_adc_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) + getFlagC() > 0xFF;
            setA(getA() + value + getFlagC());
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_adc_imm");
    }
}

void CPU::op_sub_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            bool h_bit = (getA() & 0x0F) < (value & 0x0F);
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
            setA(getA() - value);
            setFlagZ(getA() == 0);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sub_r");
    }
}

void CPU::op_sub_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) < (value & 0x0F);
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
            setA(getA() - value);
            setFlagZ(getA() == 0);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sub_hl_ind");
    }
}

void CPU::op_sub_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) < (value & 0x0F);
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
            setA(getA() - value);
            setFlagZ(getA() == 0);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sub_imm");
    }
}

void CPU::op_sbc_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF) + getFlagC();
            setA(getA() - value - getFlagC());
            setFlagZ(getA() == 0);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sbc_r");
    }
}

void CPU::op_sbc_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF) + getFlagC();
            setA(getA() - value - getFlagC());
            setFlagZ(getA() == 0);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sbc_hl_ind");
    }
}

void CPU::op_sbc_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF) + getFlagC();
            setA(getA() - value - getFlagC());
            setFlagZ(getA() == 0);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_sbc_imm");
    }
}

void CPU::op_cp_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            bool h_bit = (getA() & 0x0F) < (value & 0x0F);
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
            setFlagZ(getA() == value);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_cp_r");
    }
}

void CPU::op_cp_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) < (value & 0x0F);
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
            setFlagZ(getA() == value);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_cp_hl_ind");
    }
}

void CPU::op_cp_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            bool h_bit = (getA() & 0x0F) < (value & 0x0F);
            bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
            setFlagZ(getA() == value);
            setFlagN(1);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_cp_imm");
    }
}

void CPU::op_inc_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 5, 3);
            u8 value = read_register_8_bit(destination_register);
            value++;
            bool h_bit = (value & 0x0F) == 0x00;
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(h_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_inc_r");
    }
}

void CPU::op_inc_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            value++;
            bool h_bit = (value & 0x0F) == 0x00;
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(0);
            setFlagH(h_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_inc_hl_ind");
    }
}

void CPU::op_dec_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 destination_register = get_bits(current_opcode_, 5, 3);
            u8 value = read_register_8_bit(destination_register);
            value--;
            bool h_bit = (value & 0x0F) == 0x0F;
            write_register_8_bit(destination_register, value);
            setFlagZ(value == 0);
            setFlagN(1);
            setFlagH(h_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_dec_r");
    }
}

void CPU::op_dec_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            value--;
            bool h_bit = (value & 0x0F) == 0x0F;
            write_memory_8(hl_, value);
            setFlagZ(value == 0);
            setFlagN(1);
            setFlagH(h_bit);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_dec_hl_ind");
    }
}

void CPU::op_and_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            setA(getA() & value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(1);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_and_r");
    }
}

void CPU::op_and_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setA(getA() & value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(1);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_and_hl_ind");
    }
}

void CPU::op_and_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setA(getA() & value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(1);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_and_imm");
    }
}


void CPU::op_or_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            setA(getA() | value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_or_r");
    }
}

void CPU::op_or_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setA(getA() | value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_or_hl_ind");
    }
}

void CPU::op_or_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setA(getA() | value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_or_imm");
    }
}

void CPU::op_xor_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            setA(getA() ^ value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_xor_r");
    }
}

void CPU::op_xor_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setA(getA() ^ value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_xor_hl_ind");
    }
}

void CPU::op_xor_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 value = memory_temp_;
            setA(getA() ^ value);
            setFlagZ(getA() == 0);
            setFlagN(0);
            setFlagH(0);
            setFlagC(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_xor_imm");
    }
}

void CPU::op_ccf() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            setFlagC(!getFlagC());
            setFlagN(0);
            setFlagH(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ccf");
    }
}

void CPU::op_scf() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            setFlagC(1);
            setFlagN(0);
            setFlagH(0);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_scf");
    }
}

void CPU::op_daa() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 a = getA();
            u8 offset = 0;
            bool subtract = getFlagN();
            bool should_set_carry = false;

            if ((!subtract && (a & 0x0F) > 0x09) || getFlagH()) {
                offset |= 0x06;
            }

            if ((!subtract && a > 0x99) || getFlagC()) {
                offset |= 0x60;
                should_set_carry = true;
            }
            
            if (subtract) {
                a -= offset;
            } else {
                a += offset;
            }
            
            setA(a);
            setFlagZ(a == 0);
            setFlagH(0);
            setFlagC(should_set_carry);

            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_daa");
    }
}

void CPU::op_cpl() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            setA(~getA());
            setFlagN(1);
            setFlagH(1);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_cpl");
    }
}

// 16-bit arithmetic instructions
void CPU::op_inc_rr() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 register_number = get_bits(current_opcode_, 5, 4);
            u16 value = read_register_16_bit(register_number);
            value++;
            write_register_16_bit(register_number, value);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_inc_rr");
    }
}

void CPU::op_dec_rr() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 register_number = get_bits(current_opcode_, 5, 4);
            u16 value = read_register_16_bit(register_number);
            value--;
            write_register_16_bit(register_number, value);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_dec_rr");
    }
}

void CPU::op_add_hl_rr() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 5, 4);
            u8 value = read_register_16_bit(source_register) & 0xFF;
            u16 low_sum = static_cast<u16>(getL()) + value;
            setL(low_sum & 0xFF);
            memory_temp_ = (low_sum > 0xFF) ? 1 : 0;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER_HIGH_BYTE;
            return;
        }
        case State_Machine::ASSIGN_VALUE_TO_REGISTER_HIGH_BYTE: {
            u8 source_register = get_bits(current_opcode_, 5, 4);
            u8 value = read_register_16_bit(source_register) >> 8;
            u8 carry_in = memory_temp_ & 0xFF;
            bool h_bit = (getH() & 0x0F) + (value & 0x0F) + carry_in > 0x0F;
            bool c_bit = static_cast<u16>(getH()) + value + carry_in > 0xFF;
            setH(getH() + value + carry_in);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_add_hl_rr");
    }
}

void CPU::op_add_sp_e() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER_LOW_BYTE;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER_LOW_BYTE: {
            s8 offset = static_cast<s8>(memory_temp_);
            bool h_bit = ((sp_ & 0x0F) + (offset & 0x0F)) > 0x0F;
            bool c_bit = ((sp_ & 0xFF) + (static_cast<u8>(offset))) > 0xFF;
            setFlagZ(0);
            setFlagN(0);
            setFlagH(h_bit);
            setFlagC(c_bit);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER_HIGH_BYTE;
            return;
        }
        case State_Machine::ASSIGN_VALUE_TO_REGISTER_HIGH_BYTE: {
            s8 offset = static_cast<s8>(memory_temp_);
            memory_temp_ = sp_ + offset;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        }
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            sp_ = memory_temp_;
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_add_sp_e");
    }
}
