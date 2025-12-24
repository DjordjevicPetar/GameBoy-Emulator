#include "../inc/cpu.hpp"
#include "../inc/instruction_decoder.hpp"
#include "../inc/interrupt_controller.hpp"
#include "../inc/logger.hpp"
#include "../inc/mmu.hpp"
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
    
    InstructionDecoder::initializeHandlers(this);
}

void CPU::log(const std::string& func_name, const std::string& details) {
    Logger::log(func_name, current_opcode_, af_, bc_, de_, hl_, sp_, pc_, ime_, details);
}

uint8_t CPU::execute_next_instruction() {
    current_opcode_ = fetchOpcode();
    
    for (auto& [op, handler] : op_handlers_) {
        if ((current_opcode_ & op.mask) == op.pattern) {
            return (this->*handler)();
        }
    }
    
    std::cout << "Undefined opcode: " << std::hex << static_cast<int>(current_opcode_) << std::endl;
    throw std::runtime_error("Undefined opcode");
}

uint8_t CPU::handle_interrupts() {
    if (!ime_) {
        return 0;
    }
    
    uint16_t addr = interrupt_controller_->get_address_of_highest_priority_interrupt();
    if (addr != INTERRUPT_HANDLER_NONE_ADDRESS) {
        push_to_stack(pc_);
        pc_ = addr;
        ime_ = false;
        return 5;
    }
    return 0;
}

uint8_t CPU::fetchOpcode() {
    return mmu_->read_memory_8(pc_++);
}

uint8_t CPU::cb_ins_handler() {
    log(__func__);
    current_opcode_ = fetchOpcode();
    
    for (auto& [op, handler] : cb_handlers_) {
        if ((current_opcode_ & op.mask) == op.pattern) {
            return (this->*handler)();
        }
    }
    
    throw std::runtime_error("Undefined CB opcode");
}

uint16_t CPU::endian_swap(uint8_t low, uint8_t high) const {
    return (static_cast<uint16_t>(high) << 8) | static_cast<uint16_t>(low);
}

void CPU::push_to_stack(uint16_t value) {
    sp_ -= 2;
    mmu_->write_memory_8(sp_ + 1, value >> 8);      // HIGH byte
    mmu_->write_memory_8(sp_, value & 0xFF);        // LOW byte
}

uint16_t CPU::pop_from_stack() {
    uint16_t value = mmu_->read_memory_8(sp_) | (mmu_->read_memory_8(sp_ + 1) << 8);
    sp_ += 2;
    return value;
}

// ============================================================================
// Opcode Parameter Decoding
// ============================================================================

uint8_t CPU::read_first_register_8_bit_parameter() const {
    return (current_opcode_ >> 3) & 0x07;
}

uint8_t CPU::read_second_register_8_bit_parameter() const {
    return current_opcode_ & 0x07;
}

uint8_t CPU::read_first_register_16_bit_parameter() const {
    return (current_opcode_ >> 4) & 0x03;
}

uint8_t CPU::read_second_register_16_bit_parameter() const {
    return (current_opcode_ >> 2) & 0x03;
}

uint8_t CPU::read_bit_argument() const {
    return (current_opcode_ >> 3) & 0x07;
}

uint8_t CPU::read_condition_argument() const {
    uint8_t cond = (current_opcode_ >> 3) & 0x03;
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

uint8_t CPU::read_register_8_bit(uint8_t reg_num) const {
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

void CPU::write_register_8_bit(uint8_t reg_num, uint8_t value) {
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

uint16_t CPU::read_register_16_bit(uint8_t reg_num) const {
    switch (reg_num) {
        case 0: return bc_;
        case 1: return de_;
        case 2: return hl_;
        case 3: return sp_;
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit(uint8_t reg_num, uint16_t value) {
    switch (reg_num) {
        case 0: bc_ = value; break;
        case 1: de_ = value; break;
        case 2: hl_ = value; break;
        case 3: sp_ = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

uint16_t CPU::read_register_16_bit_stack(uint8_t reg_num) const {
    switch (reg_num) {
        case 0: return bc_;
        case 1: return de_;
        case 2: return hl_;
        case 3: return af_;
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit_stack(uint8_t reg_num, uint16_t value) {
    switch (reg_num) {
        case 0: bc_ = value; break;
        case 1: de_ = value; break;
        case 2: hl_ = value; break;
        case 3: af_ = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

uint16_t CPU::read_register_16_bit_memory(uint8_t reg_num) {
    switch (reg_num) {
        case 0: return bc_;
        case 1: return de_;
        case 2: return hl_++;
        case 3: return hl_--;
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit_memory(uint8_t reg_num, uint16_t value) {
    switch (reg_num) {
        case 0: bc_ = value; break;
        case 1: de_ = value; break;
        case 2: hl_ = value; hl_++; break;
        case 3: hl_ = value; hl_--; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

// ============================================================================
// 8-bit Load Instructions
// ============================================================================

uint8_t CPU::op_ld_r_r() {
    log(__func__);
    uint8_t src = read_second_register_8_bit_parameter();
    uint8_t dst = read_first_register_8_bit_parameter();
    write_register_8_bit(dst, read_register_8_bit(src));
    return 4;
}

uint8_t CPU::op_ld_r_imm() {
    log(__func__);
    uint8_t dst = read_first_register_8_bit_parameter();
    write_register_8_bit(dst, fetchOpcode());
    return 8;
}

uint8_t CPU::op_ld_r_hl_ind() {
    log(__func__);
    uint8_t dst = read_first_register_8_bit_parameter();
    write_register_8_bit(dst, mmu_->read_memory_8(hl_));
    return 8;
}

uint8_t CPU::op_ld_hl_ind_r() {
    log(__func__);
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint16_t address = hl_;
    uint8_t value = read_register_8_bit(source_register);
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_imm() {
    log(__func__);
    uint16_t address = hl_;
    uint8_t value = fetchOpcode();
    mmu_->write_memory_8(address, value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_a_bc_ind() {
    log(__func__);
    uint16_t address = bc_;
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_a_de_ind() {
    log(__func__);
    uint16_t address = de_;
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_bc_ind_a() {
    log(__func__);
    uint16_t address = bc_;
    uint8_t value = getA();
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_de_ind_a() {
    log(__func__);
    uint16_t address = de_;
    uint8_t value = getA();
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_a_imm_ind() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    return 16; // 16 cycles
}

uint8_t CPU::op_ld_imm_ind_a() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    uint8_t value = getA();
    mmu_->write_memory_8(address, value);
    return 16; // 16 cycles
}

uint8_t CPU::op_ldh_a_c_ind() {
    log(__func__);
    uint16_t address = 0xFF00 + getC();
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ldh_c_ind_a() {
    log(__func__);
    uint16_t address = 0xFF00 + getC();
    uint8_t value = getA();
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_ldh_a_imm_ind() {
    log(__func__);
    uint16_t address = 0xFF00 + fetchOpcode();
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ldh_imm_ind_a() {
    log(__func__);
    uint8_t value = getA();
    uint16_t address = 0xFF00 + fetchOpcode();
    mmu_->write_memory_8(address, value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_a_hl_ind_dec() {
    log(__func__);
    uint16_t address = hl_;
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    hl_--;
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_dec_a() {
    log(__func__);
    uint16_t address = hl_;
    uint8_t value = getA();
    mmu_->write_memory_8(address, value);
    hl_--;
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_a_hl_ind_inc() {
    log(__func__);
    uint16_t address = hl_;
    uint8_t value = mmu_->read_memory_8(address);
    setA(value);
    hl_++;
    return 8; // 8 cycles
}

uint8_t CPU::op_ld_hl_ind_inc_a() {
    log(__func__);
    uint16_t address = hl_;
    uint8_t value = getA();
    mmu_->write_memory_8(address, value);
    hl_++;
    return 8; // 8 cycles
}

// 16-bit load instructions
uint8_t CPU::op_ld_rr_imm() {
    log(__func__);
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = endian_swap(fetchOpcode(), fetchOpcode());
    write_register_16_bit(register_number, value);
    return 12; // 12 cycles
}

uint8_t CPU::op_ld_imm_ind_sp() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    mmu_->write_memory_8(address, sp_ & 0xFF);
    mmu_->write_memory_8(address + 1, sp_ >> 8);
    return 16; // 16 cycles
}

uint8_t CPU::op_ld_sp_hl() {
    log(__func__);
    sp_ = hl_;
    return 8; // 8 cycles
}

uint8_t CPU::op_push_rr() {
    log(__func__);
    uint8_t register_number = read_first_register_16_bit_parameter();
    push_to_stack(read_register_16_bit_stack(register_number));
    return 16;
}

uint8_t CPU::op_pop_rr() {
    log(__func__);
    uint8_t register_number = read_first_register_16_bit_parameter();
    write_register_16_bit_stack(register_number, pop_from_stack());
    return 12;
}

uint8_t CPU::op_ld_hl_sp_e() {
    log(__func__);
    uint8_t value = fetchOpcode();
    bool h_bit;
    bool c_bit;
    if (value >> 7) {
        value = ~value + 1;
        hl_ = sp_ - value;
        h_bit = (sp_ & 0x0F) < (value & 0x0F);
        c_bit = (sp_ & 0xFF) < (value & 0xFF);
    } else {
        hl_ = sp_ + value;
        h_bit = (sp_ & 0x0F) + (value & 0x0F) > 0x0F;
        c_bit = (sp_ & 0xFF) + (value & 0xFF) > 0xFF;
    }
    setFlagZ(0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

// 8-bit arithmetic and logical instructions
uint8_t CPU::op_add_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
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
    log(__func__);
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
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
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
    log(__func__);
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
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
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
    log(__func__);
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
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF) + getFlagC();
    setA(getA() - value - getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_sbc_imm() {
    log(__func__);
    uint8_t value = fetchOpcode();
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
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<uint16_t>(getA()) & 0xFF) < (static_cast<uint16_t>(value) & 0xFF);
    setFlagZ(getA() == value);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_cp_imm() {
    log(__func__);
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
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    value++;
    bool h_bit = (value & 0x0F) == 0x0F;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(h_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_dec_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    value--;
    bool h_bit = (value & 0x0F) == 0x00;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(1);
    setFlagH(h_bit);
    return 12; // 12 cycles
}

uint8_t CPU::op_and_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_and_imm() {
    log(__func__);
    uint8_t value = fetchOpcode();
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_or_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_or_imm() {
    log(__func__);
    uint8_t value = fetchOpcode();
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_xor_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_xor_imm() {
    log(__func__);
    uint8_t value = fetchOpcode();
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

uint8_t CPU::op_ccf() {
    log(__func__);
    setFlagC(!getFlagC());
    setFlagN(0);
    setFlagH(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_scf() {
    log(__func__);
    setFlagC(1);
    setFlagN(0);
    setFlagH(0);
    return 4; // 4 cycles
}

uint8_t CPU::op_daa() {
    log(__func__);
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
    log(__func__);
    setA(~getA());
    setFlagN(1);
    setFlagH(1);
    return 4; // 4 cycles
}

// 16-bit arithmetic instructions
uint8_t CPU::op_inc_rr() {
    log(__func__);
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit(register_number);
    value++;
    write_register_16_bit(register_number, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_dec_rr() {
    log(__func__);
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit(register_number);
    value--;
    write_register_16_bit(register_number, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_add_hl_rr() {
    log(__func__);
    uint8_t source_register = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit(source_register);
    bool h_bit = (hl_ & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (hl_ & 0xFF) + (value & 0xFF) > 0xFF;
    hl_ = hl_ + value;
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

uint8_t CPU::op_add_sp_e() {
    log(__func__);
    uint8_t value = fetchOpcode();
    bool h_bit;
    bool c_bit;
    if (value >> 7) {
        value = ~value + 1;
        sp_ = sp_ - value;
        h_bit = (sp_ & 0x0F) < (value & 0x0F);
        c_bit = (sp_ & 0xFF) < (value & 0xFF);
    } else {
        sp_ = sp_ + value;
        h_bit = (sp_ & 0x0F) + (value & 0x0F) > 0x0F;
        c_bit = (sp_ & 0xFF) + (value & 0xFF) > 0xFF;
    }
    setFlagZ(0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

// Rotate, shift, and bit operation instructions
uint8_t CPU::op_rlca() {
    log(__func__);
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
    log(__func__);
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
    log(__func__);
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
    log(__func__);
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
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_rrc_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (c_bit << 7);
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_rl_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_rr_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (getFlagC() << 7);
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_sla_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value >> 7;
    value = value << 1;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_sra_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = value >> 1;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_swap_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    value = (value << 4) | (value >> 4);
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 16; // 16 cycles
}

uint8_t CPU::op_srl_r() {
    log(__func__);
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
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = value >> 1;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

uint8_t CPU::op_bit_b_r() {
    log(__func__);
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    bool bit = value & (1 << read_bit_argument());
    setFlagZ(bit == 0);
    setFlagN(0);
    setFlagH(1);
    return 8; // 8 cycles
}

uint8_t CPU::op_bit_b_hl_ind() {
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    bool bit = value & (1 << read_bit_argument());
    setFlagZ(bit == 0);
    setFlagN(0);
    setFlagH(1);
    return 12; // 12 cycles
}

uint8_t CPU::op_res_b_r() {
    log(__func__);
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    value = value & ~(1 << read_bit_argument());
    write_register_8_bit(source_register, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_res_b_hl_ind() {
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    value = value & ~(1 << read_bit_argument());
    mmu_->write_memory_8(hl_, value);
    return 16; // 16 cycles
}

uint8_t CPU::op_set_b_r() {
    log(__func__);
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    value = value | (1 << read_bit_argument());
    write_register_8_bit(source_register, value);
    return 8; // 8 cycles
}

uint8_t CPU::op_set_b_hl_ind() {
    log(__func__);
    uint8_t value = mmu_->read_memory_8(hl_);
    value = value | (1 << read_bit_argument());
    mmu_->write_memory_8(hl_, value);
    return 16; // 16 cycles
}


// Control flow instructions
uint8_t CPU::op_jp_imm() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    pc_ = address;
    return 16; // 16 cycles
}

uint8_t CPU::op_jp_hl() {
    log(__func__);
    pc_ = hl_;
    return 4; // 4 cycles
}

uint8_t CPU::op_jp_cc_imm() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    if (read_condition_argument()) {
        pc_ = address;
        return 16; // 16 cycles
    }
    return 12; // 12 cycles
}

uint8_t CPU::op_jr_e() {
    log(__func__);
    uint8_t value = fetchOpcode();
    if (value >> 7) {
        value = ~value + 1;
        pc_ = pc_ - value;
    } else {
        pc_ = pc_ + value;
    }
    return 12; // 12 cycles
}

uint8_t CPU::op_jr_cc_e() {
    log(__func__);
    uint8_t value = fetchOpcode();
    if (read_condition_argument()) {
        if (value >> 7) {
            value = ~value + 1;
            pc_ = pc_ - value;
        } else {
            pc_ = pc_ + value;
        }
        return 12; // 12 cycles
    }
    return 8; // 8 cycles
}

uint8_t CPU::op_call_imm() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    push_to_stack(pc_);
    pc_ = address;
    return 24;
}

uint8_t CPU::op_call_cc_imm() {
    log(__func__);
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    if (read_condition_argument()) {
        push_to_stack(pc_);
        pc_ = address;
        return 24;
    }
    return 12;
}

uint8_t CPU::op_ret() {
    log(__func__);
    pc_ = pop_from_stack();
    return 16;
}

uint8_t CPU::op_ret_cc() {
    log(__func__);
    if (read_condition_argument()) {
        pc_ = pop_from_stack();
        return 20;
    }
    return 8;
}

uint8_t CPU::op_reti() {
    log(__func__);
    pc_ = pop_from_stack();
    ime_ = true;
    return 16;
}

uint8_t CPU::op_rst_imm() {
    log(__func__);
    push_to_stack(pc_);
    pc_ = read_bit_argument() << 3;
    return 16;
}

// Miscellaneous instructions
uint8_t CPU::op_halt() {
    log(__func__);
    // HALT instruction - CPU stops until interrupt
    // GameBoyEmulator will handle the stop condition
    return 4; // 4 cycles
}

uint8_t CPU::op_stop() {
    log(__func__);
    // STOP instruction - CPU and GPU stop
    // GameBoyEmulator will handle the stop condition
    mmu_->write_memory_8(DIV_REGISTER_LOCATION, 0x00);
    return 4; // 4 cycles
}

uint8_t CPU::op_di() {
    log(__func__);
    ime_ = false;
    // TODO: Add interrupt handling
    return 4; // 4 cycles
}

uint8_t CPU::op_ei() {
    log(__func__);
    ime_ = true;
    return 4; // 4 cycles
}

uint8_t CPU::op_nop() {
    log(__func__);
    return 4; // 4 cycles
}