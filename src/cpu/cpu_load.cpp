#include "cpu.hpp"
#include "../memory/mmu.hpp"

// ============================================================================
// 8-bit Load Instructions
// ============================================================================

void CPU::op_ld_r_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 src = get_bits(current_opcode_, 2, 0);
            u8 dst = get_bits(current_opcode_, 5, 3);
            u8 value = read_register_8_bit(src);
            write_register_8_bit(dst, value);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_ld_r_r");
    }
}

void CPU::op_ld_r_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 dst = get_bits(current_opcode_, 5, 3);
            write_register_8_bit(dst, memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_ld_r_imm");
    }
}

void CPU::op_ld_r_hl_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 dst = get_bits(current_opcode_, 5, 3);
            write_register_8_bit(dst, memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_ld_r_hl_ind");
    }
}

void CPU::op_ld_hl_ind_r() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE: {
            u8 source_register = get_bits(current_opcode_, 2, 0);
            u8 value = read_register_8_bit(source_register);
            write_memory_8(hl_, value);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_hl_ind_r");
    }
}

void CPU::op_ld_hl_ind_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY:
            write_memory_8(hl_, memory_temp_);
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_hl_ind_imm");
    }
}

void CPU::op_ld_a_bc_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(bc_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_a_bc_ind");
    }
}

void CPU::op_ld_a_de_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(de_);
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_a_de_ind");
    }
}

void CPU::op_ld_bc_ind_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            write_memory_8(bc_, getA());
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_bc_ind_a");
    }
}

void CPU::op_ld_de_ind_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            write_memory_8(de_, getA());
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_de_ind_a");
    }
}

void CPU::op_ld_a_imm_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY;
            return;
        case State_Machine::READ_MEMORY:
            read_memory_8(memory_temp_);
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_a_imm_ind");
    }
}

void CPU::op_ld_imm_ind_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY:
            write_memory_8(memory_temp_, getA());
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_imm_ind_a");
    }
}

void CPU::op_ldh_a_c_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(0xFF00 + getC());
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ldh_a_c_ind");
    }
}

void CPU::op_ldh_c_ind_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            write_memory_8(0xFF00 + getC(), getA());
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ldh_c_ind_a");
    }
}

void CPU::op_ldh_a_imm_ind() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY:
            read_memory_8(0xFF00 + (memory_temp_ & 0xFF));
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ldh_a_imm_ind");
    }
}

void CPU::op_ldh_imm_ind_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY:
            write_memory_8(0xFF00 + (memory_temp_ & 0xFF), getA());
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ldh_imm_ind_a");
    }
}

void CPU::op_ld_a_hl_ind_dec() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            hl_--;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_a_hl_ind_dec");
    }
}

void CPU::op_ld_hl_ind_dec_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            write_memory_8(hl_, getA());
            hl_--;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_hl_ind_dec_a");
    }
}

void CPU::op_ld_a_hl_ind_inc() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(hl_);
            hl_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER:
            setA(memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_a_hl_ind_inc");
    }
}

void CPU::op_ld_hl_ind_inc_a() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            write_memory_8(hl_, getA());
            hl_++;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_hl_ind_inc_a");
    }
}

// 16-bit load instructions
void CPU::op_ld_rr_imm() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY: {
            u8 register_number = get_bits(current_opcode_, 5, 4);
            write_register_16_bit(register_number, memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_ld_rr_imm");
    }
}

void CPU::op_ld_imm_ind_sp() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::READ_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::READ_MEMORY_HIGH_BYTE:
            read_memory_8(pc_, true);
            pc_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY_LOW_BYTE;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY_LOW_BYTE:
            write_memory_8(memory_temp_, sp_ & 0xFF);
            memory_temp_++;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY_HIGH_BYTE:
            write_memory_8(memory_temp_, sp_ >> 8);
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_imm_ind_sp");
    }
}

void CPU::op_ld_sp_hl() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            sp_ = hl_;
            state_machine_ = State_Machine::NO_OP;
            return;
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_ld_sp_hl");
    }
}

void CPU::op_push_rr() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            sp_--;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY_HIGH_BYTE;
            return;
        case State_Machine::WRITE_VALUE_TO_MEMORY_HIGH_BYTE: {
            u8 register_number = get_bits(current_opcode_, 5, 4);
            write_memory_8(sp_, read_register_16_bit_stack(register_number) >> 8);
            sp_--;
            state_machine_ = State_Machine::WRITE_VALUE_TO_MEMORY_LOW_BYTE;
            return;
        }
        case State_Machine::WRITE_VALUE_TO_MEMORY_LOW_BYTE: {
            u8 register_number = get_bits(current_opcode_, 5, 4);
            write_memory_8(sp_, read_register_16_bit_stack(register_number) & 0xFF);
            state_machine_ = State_Machine::NO_OP;
            return;
        }
        case State_Machine::NO_OP:
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        default:
            throw std::runtime_error("Invalid state in op_push_rr");
    }
}

void CPU::op_pop_rr() {
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
        case State_Machine::ASSIGN_VALUE_TO_REGISTER: {
            u8 register_number = get_bits(current_opcode_, 5, 4);
            write_register_16_bit_stack(register_number, memory_temp_);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_pop_rr");
    }
}

void CPU::op_ld_hl_sp_e() {
    switch (state_machine_) {
        case State_Machine::INITIAL_PHASE:
            read_memory_8(pc_);
            pc_++;
            state_machine_ = State_Machine::ASSIGN_VALUE_TO_REGISTER_LOW_BYTE;
            return;
        case State_Machine::ASSIGN_VALUE_TO_REGISTER_LOW_BYTE: {
            s8 offset = static_cast<s8>(memory_temp_);
            setL((sp_ + offset) & 0xFF);
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
            setH((sp_ + offset) >> 8);
            state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;
            return;
        }
        default:
            throw std::runtime_error("Invalid state in op_ld_hl_sp_e");
    }
}
