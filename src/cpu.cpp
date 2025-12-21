#include "../inc/cpu.hpp"
#include "../inc/instruction_decoder.hpp"
#include <iostream>

CPU::CPU(MMU* mmu) : mmu(mmu) {
    this->IME = false;
    
    // Initialize registers
    setA(0x01);
    setB(0x00);
    setC(0x13);
    setD(0x00);
    setE(0xD8);
    setH(0x01);
    setL(0x4D);
    PC = 0x0100;
    SP = 0xFFFE;
    
    // Initialize flags
    setFlagZ(true);
    setFlagN(false);
    setFlagH(true);
    setFlagC(true);
    
    // Initialize instruction handlers - will be called with this CPU instance
    InstructionDecoder::initializeHandlers(this);
}

uint8_t CPU::execute_next_instruction() {
    current_opcode = fetchOpcode();
    bool is_recognized = false;
    
    for (auto& entry : op_handlers) {
        if ((current_opcode & entry.first.mask) == entry.first.pattern) {
            uint8_t cycles = (this->*(entry.second))();
            is_recognized = true;
            return cycles;
        }
    }
    
    if (!is_recognized) {
        std::cout << "Undefined opcode: " << std::hex << (int)current_opcode << std::endl;
        throw std::runtime_error("Undefined opcode");
    }
    
    return 0;
}

uint8_t CPU::fetchOpcode() {
    uint8_t opcode = mmu->read_memory_8(PC);
    PC++;
    return opcode;
}

uint8_t CPU::cb_ins_handler() {
    // Read the next byte after 0xCB (the actual CB instruction opcode)
    current_opcode = fetchOpcode();
    bool is_recognized = false;
    uint8_t cycles = 0;
    
    for (auto& entry : cb_handlers) {
        if ((current_opcode & entry.first.mask) == entry.first.pattern) {
            cycles = (this->*(entry.second))();
            is_recognized = true;
            break;
        }
    }
    
    if (!is_recognized) {
        throw std::runtime_error("Undefined CB opcode");
    }
    
    return cycles;
}

uint16_t CPU::endian_swap(uint8_t value1, uint8_t value2) const {
    return (static_cast<uint16_t>(value2) << 8) | static_cast<uint16_t>(value1);
}

inline uint8_t CPU::read_first_register_8_bit_parameter() const {
    // Source register is in bits 5-3
    return (current_opcode >> 3) & 0x07;
}

inline uint8_t CPU::read_second_register_8_bit_parameter() const {
    // Destination register is in bits 2-0
    return current_opcode & 0x07;
}

inline uint8_t CPU::read_first_register_16_bit_parameter() const {
    // Source register is in bits 5-4
    return (current_opcode >> 4) & 0x03;
}

inline uint8_t CPU::read_second_register_16_bit_parameter() const {
    // Destination register is in bits 3-2
    return (current_opcode >> 2) & 0x03;
}

inline uint8_t CPU::read_register_8_bit(uint8_t register_number) const {
    switch (register_number) {
        case 0: return getB();
        case 1: return getC();
        case 2: return getD();
        case 3: return getE();
        case 4: return getH();
        case 5: return getL();
        case 6: return mmu->read_memory_8(HL);  // (HL) - indirect
        case 7: return getA();
        default: throw std::runtime_error("Invalid register number");
    }
}

inline void CPU::write_register_8_bit(uint8_t register_number, uint8_t value) {
    switch (register_number) {
        case 0: setB(value); break;
        case 1: setC(value); break;
        case 2: setD(value); break;
        case 3: setE(value); break;
        case 4: setH(value); break;
        case 5: setL(value); break;
        case 6: mmu->write_memory_8(HL, value); break;  // (HL) - indirect
        case 7: setA(value); break;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline uint16_t CPU::read_register_16_bit(uint8_t register_number) const {
    switch (register_number) {
        case 0: return BC;
        case 1: return DE;
        case 2: return HL;
        case 3: return SP;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline void CPU::write_register_16_bit(uint8_t register_number, uint16_t value) {
    switch (register_number) {
        case 0: BC = value; break;
        case 1: DE = value; break;
        case 2: HL = value; break;
        case 3: SP = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline uint16_t CPU::read_register_16_bit_stack(uint8_t register_number) const {
    switch (register_number) {
        case 0: return BC;
        case 1: return DE;
        case 2: return HL;
        case 3: return AF;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline void CPU::write_register_16_bit_stack(uint8_t register_number, uint16_t value) {
    switch (register_number) {
        case 0: BC = value; break;
        case 1: DE = value; break;
        case 2: HL = value; break;
        case 3: AF = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

uint16_t CPU::read_register_16_bit_memory(uint8_t register_number) {
    switch (register_number) {
        case 0: return BC;
        case 1: return DE;
        case 2: {
            uint16_t value = HL;
            HL++;
            return value;
        }
        case 3: {
            uint16_t value = HL;
            HL--;
            return value;
        }
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit_memory(uint8_t register_number, uint16_t value) {
    switch (register_number) {
        case 0: BC = value; break;
        case 1: DE = value; break;
        case 2: HL = value; HL++; break;
        case 3: HL = value; HL--; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

uint8_t CPU::read_bit_argument() const {
    return (current_opcode >> 3) & 0x07;
}

uint8_t CPU::read_condition_argument() const {
    uint8_t condition = (current_opcode >> 3) & 0x03;
    switch (condition) {
        case 0: return getFlagN() & getFlagZ();
        case 1: return getFlagZ();
        case 2: return getFlagN() & getFlagC();
        case 3: return getFlagC();
        default: throw std::runtime_error("Invalid condition");
    }
}

// 8-bit load instructions
uint8_t CPU::op_ld_r_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    write_register_8_bit(destination_register, value);
    return 4; // 4 cycles
}

uint8_t CPU::op_ld_r_imm() {
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint8_t value = fetchOpcode();
    write_register_8_bit(destination_register, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_r_hl_ind() {
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint16_t address = HL;
    uint8_t value = mmu->read_memory_8(address);
    write_register_8_bit(destination_register, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint16_t address = HL;
    uint8_t value = read_register_8_bit(source_register);
    mmu->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_imm() {
    uint16_t address = HL;
    uint8_t value = fetchOpcode();
    mmu->write_memory_8(address, value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_a_bc_ind() {
    uint16_t address = BC;
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_a_de_ind() {
    uint16_t address = DE;
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_bc_ind_a() {
    uint16_t address = BC;
    uint8_t value = getA();
    mmu->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_de_ind_a() {
    uint16_t address = DE;
    uint8_t value = getA();
    mmu->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_a_imm_ind() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    return 16; // 16 cycles
}

uint8_t CPU::op_ld_imm_ind_a() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    uint8_t value = getA();
    mmu->write_memory_8(address, value);
    return 16; // 16 cycles
}

uint8_t CPU::op_ldh_a_c_ind() {
    uint16_t address = 0xFF00 + getC();
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ldh_c_ind_a() {
    uint16_t address = 0xFF00 + getC();
    uint8_t value = getA();
    mmu->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ldh_a_imm_ind() {
    uint16_t address = 0xFF00 + fetchOpcode();
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ldh_imm_ind_a() {
    uint8_t value = getA();
    uint16_t address = 0xFF00 + fetchOpcode();
    mmu->write_memory_8(address, value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_a_hl_ind_dec() {
    uint16_t address = HL;
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    HL--;
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_dec_a() {
    uint16_t address = HL;
    uint8_t value = getA();
    mmu->write_memory_8(address, value);
    HL--;
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_a_hl_ind_inc() {
    uint16_t address = HL;
    uint8_t value = mmu->read_memory_8(address);
    setA(value);
    HL++;
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_inc_a() {
    uint16_t address = HL;
    uint8_t value = getA();
    mmu->write_memory_8(address, value);
    HL++;
    return 8; // 8 cycles
}

// 16-bit load instructions
uint8_t CPU::op_ld_rr_imm() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = endian_swap(fetchOpcode(), fetchOpcode());
    write_register_16_bit(register_number, value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_imm_ind_sp() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    mmu->write_memory_8(address, SP & 0xFF);
    mmu->write_memory_8(address + 1, SP >> 8);
    return 16; // 16 cycles
}

uint8_t CPU::op_ld_sp_hl() {
    SP = HL;
    return 8; // 8 cycles
}

uint8_t CPU::op_push_rr() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit_stack(register_number);
    mmu->write_memory_8(SP - 1, value & 0xFF);
    mmu->write_memory_8(SP - 2, value >> 8);
    SP -= 2;
    return 16; // 16 cycles
}

uint8_t CPU::op_pop_rr() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = endian_swap(mmu->read_memory_8(SP), mmu->read_memory_8(SP + 1));
    write_register_16_bit_stack(register_number, value);
    SP += 2;
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_hl_sp_e() {
    uint8_t value = fetchOpcode();
    bool h_bit;
    bool c_bit;
    if (value >> 7) {
        value = ~value + 1;
        HL = SP - value;
        h_bit = (SP & 0x0F) < (value & 0x0F);
        c_bit = (SP & 0xFF) < (value & 0xFF);
    } else {
        HL = SP + value;
        h_bit = (SP & 0x0F) + (value & 0x0F) > 0x0F;
        c_bit = (SP & 0xFF) + (value & 0xFF) > 0xFF;
    }
    setFlagZ(0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

// 8-bit arithmetic and logical instructions
uint8_t CPU::op_add_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) + (static_cast<uint16_t>(value) & 0xFF) > 0xFF;
    setA(getA() + value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_add_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) + (static_cast<uint16_t>(value) & 0xFF) > 0xFF;
    setA(getA() + value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_add_imm() {
    uint8_t value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) + (static_cast<uint16_t>(value) & 0xFF) > 0xFF;
    setA(getA() + value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_adc_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) + (static_cast<uint16_t>(value) & 0xFF) + getFlagC() > 0xFF;
    setA(getA() + value + getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_adc_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) + (static_cast<uint16_t>(value) & 0xFF) + getFlagC() > 0xFF;
    setA(getA() + value + getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_adc_imm() {
    uint8_t value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) + (static_cast<uint16_t>(value) & 0xFF) + getFlagC() > 0xFF;
    setA(getA() + value + getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sub_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setA(getA() - value);
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_sub_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setA(getA() - value);
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sub_imm() {
    uint8_t value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setA(getA() - value);
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sbc_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF) + getFlagC();
    setA(getA() - value - getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_sbc_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF) + getFlagC();
    setA(getA() - value - getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sbc_imm() {    uint8_t value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF) + getFlagC();
    setA(getA() - value - getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_cp_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setFlagZ(getA() == value);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_cp_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setFlagZ(getA() == value);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_cp_imm() {
    uint8_t value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setFlagZ(getA() == value);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_inc_r() {
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    value++;
    bool h_bit = (value & 0x0F) == 0x0F;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(h_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_inc_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    value++;
    bool h_bit = (value & 0x0F) == 0x0F;
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(h_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_dec_r() {
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    value--;
    bool h_bit = (value & 0x0F) == 0x00;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(1);
    setFlagH(h_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_dec_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    value--;
    bool h_bit = (value & 0x0F) == 0x00;
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(1);
    setFlagH(h_bit);
    return 12; // 12 cycles
}

uint8_t CPU::op_and_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_and_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_and_imm() {
    uint8_t value = fetchOpcode();
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_or_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_or_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_or_imm() {
    uint8_t value = fetchOpcode();
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_xor_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_xor_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_xor_imm() {
    uint8_t value = fetchOpcode();
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_ccf() {
    setFlagC(!getFlagC());
    setFlagN(0);
    setFlagH(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_scf() {
    setFlagC(1);
    setFlagN(0);
    setFlagH(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_daa() {
    if (getFlagN() == 0) {
        if (getFlagH() == 1 || (getA() & 0x0F) > 0x09) {
            setA(getA() + 0x06);
        }
        if (getFlagC() == 1 || getA() > 0x99) {
            setA(getA() + 0x60);
        }
    } else {
        if (getFlagH() == 1) {
            setA(getA() - 0x06);
        }
        if (getFlagC() == 1) {
            setA(getA() - 0x60);
        }
    }
    setFlagZ(getA() == 0);
    setFlagH(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_cpl() {
    setA(~getA());
    setFlagN(1);
    setFlagH(1);
    return 4; // 4 cycles
}

// 16-bit arithmetic instructions
uint8_t CPU::op_inc_rr() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit(register_number);
    value++;
    write_register_16_bit(register_number, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_dec_rr() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit(register_number);
    value--;
    write_register_16_bit(register_number, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_add_hl_rr() {
    uint8_t source_register = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit(source_register);
    bool h_bit = (HL & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (HL & 0xFF) + (value & 0xFF) > 0xFF;
    HL = HL + value;
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_add_sp_e() {
    uint8_t value = fetchOpcode();
    bool h_bit;
    bool c_bit;
    if (value >> 7) {
        value = ~value + 1;
        SP = SP - value;
        h_bit = (SP & 0x0F) < (value & 0x0F);
        c_bit = (SP & 0xFF) < (value & 0xFF);
    } else {
        SP = SP + value;
        h_bit = (SP & 0x0F) + (value & 0x0F) > 0x0F;
        c_bit = (SP & 0xFF) + (value & 0xFF) > 0xFF;
    }
    setFlagZ(0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

// Rotate, shift, and bit operation instructions
uint8_t CPU::op_rlca() {
    uint8_t value = getA();
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_rrca() {
    uint8_t value = getA();
    bool c_bit = value & 0x01;
    value = (value >> 1) | c_bit;
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_rla() {
    uint8_t value = getA();
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

uint8_t CPU::op_rra() {
    uint8_t value = getA();
    bool c_bit = value & 0x01;
    value = (value >> 1) | (getFlagC() << 7);
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

// CB prefix instructions
uint8_t CPU::op_rlc_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_rlc_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_rrc_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (c_bit << 7);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_rrc_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (c_bit << 7);
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_rl_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_rl_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_rr_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (getFlagC() << 7);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_rr_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (getFlagC() << 7);
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_sla_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value >> 7;
    value = value << 1;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sla_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value >> 7;
    value = value << 1;
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_sra_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (value & 0x80);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sra_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value & 0x01;
    value = value >> 1;
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_swap_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    value = (value << 4) | (value >> 4);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_swap_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    value = (value << 4) | (value >> 4);
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 16; // 16 cycles
}

uint8_t CPU::op_srl_r() {
    uint8_t destination_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = value >> 1;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_srl_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool c_bit = value & 0x01;
    value = value >> 1;
    mmu->write_memory_8(HL, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_bit_b_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool bit = value & (1 << read_bit_argument());
    setFlagZ(bit == 0);
    setFlagN(0);
    setFlagH(1);
    return 8; // 8 cycles
}

uint8_t CPU::op_bit_b_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    bool bit = value & (1 << read_bit_argument());
    setFlagZ(bit == 0);
    setFlagN(0);
    setFlagH(1);
    return 12; // 12 cycles
}

uint8_t CPU::op_res_b_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    value = value & ~(1 << read_bit_argument());
    write_register_8_bit(source_register, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_res_b_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    value = value & ~(1 << read_bit_argument());
    mmu->write_memory_8(HL, value);
    return 16; // 16 cycles
}

uint8_t CPU::op_set_b_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    value = value | (1 << read_bit_argument());
    write_register_8_bit(source_register, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_set_b_hl_ind() {
    uint8_t value = mmu->read_memory_8(HL);
    value = value | (1 << read_bit_argument());
    mmu->write_memory_8(HL, value);
    return 16; // 16 cycles
}


// Control flow instructions
uint8_t CPU::op_jp_imm() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    PC = address;
    return 16; // 16 cycles
}

uint8_t CPU::op_jp_hl() {
    PC = HL;
    return 4; // 4 cycles
}

uint8_t CPU::op_jp_cc_imm() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    if (read_condition_argument()) {
        PC = address;
        return 16; // 16 cycles
    }
    return 12; // 12 cycles
}

uint8_t CPU::op_jr_e() {
    uint8_t value = fetchOpcode();
    if (value >> 7) {
        value = ~value + 1;
        PC = PC - value;
    } else {
        PC = PC + value;
    }
    return 12; // 12 cycles
}

uint8_t CPU::op_jr_cc_e() {
    uint8_t value = fetchOpcode();
    if (read_condition_argument()) {
        if (value >> 7) {
            value = ~value + 1;
            PC = PC - value;
        } else {
            PC = PC + value;
        }
        return 12; // 12 cycles
    }
    return 8; // 8 cycles
}

uint8_t CPU::op_call_imm() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    mmu->write_memory_8(SP - 1, PC & 0xFF);
    mmu->write_memory_8(SP - 2, PC >> 8);
    SP -= 2;
    PC = address;
    return 24; // 24 cycles
}

uint8_t CPU::op_call_cc_imm() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    if (read_condition_argument()) {
        mmu->write_memory_8(SP - 1, PC & 0xFF);
        mmu->write_memory_8(SP - 2, PC >> 8);
        SP -= 2;
        PC = address;
        return 24; // 24 cycles
    }
    return 12; // 12 cycles
}

uint8_t CPU::op_ret() {
    uint16_t address = mmu->read_memory_8(SP) | (mmu->read_memory_8(SP + 1) << 8);
    SP += 2;
    PC = address;
    return 16; // 16 cycles
}

uint8_t CPU::op_ret_cc() {
    if (read_condition_argument()) {
        uint16_t address = mmu->read_memory_8(SP) | (mmu->read_memory_8(SP + 1) << 8);
        SP += 2;
        PC = address;
        return 20; // 20 cycles
    }
    return 8; // 8 cycles
}

uint8_t CPU::op_reti() {
    uint16_t address = mmu->read_memory_8(SP) | (mmu->read_memory_8(SP + 1) << 8);
    SP += 2;
    PC = address;
    IME = true;
    return 16;
}

uint8_t CPU::op_rst_imm() {
    mmu->write_memory_8(SP - 1, PC & 0xFF);
    mmu->write_memory_8(SP - 2, PC >> 8);
    SP -= 2;
    PC = read_bit_argument() << 3;
    return 16; // 16 cycles
}

// Miscellaneous instructions
uint8_t CPU::op_halt() {
    // HALT instruction - CPU stops until interrupt
    // GameBoyEmulator will handle the stop condition
    return 4; // 4 cycles
}

uint8_t CPU::op_stop() {
    // STOP instruction - CPU and GPU stop
    // GameBoyEmulator will handle the stop condition
    mmu->write_memory_8(DIV_REGISTER_LOCATION, 0x00);
    return 4; // 4 cycles
}

uint8_t CPU::op_di() {
    IME = false;
    // TODO: Add interrupt handling
    return 4; // 4 cycles
}

uint8_t CPU::op_ei() {
    IME = true;
    return 4; // 4 cycles
}

uint8_t CPU::op_nop() {
    return 4; // 4 cycles
}