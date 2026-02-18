#include "cpu.hpp"
#include "instruction_decoder.hpp"
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
    
    InstructionDecoder::initializeHandlers(this);
}

void CPU::log(const std::string& func_name, const std::string& details) {
    (void)func_name; (void)details; // Unused in EpicLog format
    
    u8 A = (af_ >> 8) & 0xFF;
    u8 F = af_ & 0xFF;
    u8 B = (bc_ >> 8) & 0xFF;
    u8 C = bc_ & 0xFF;
    u8 D = (de_ >> 8) & 0xFF;
    u8 E = de_ & 0xFF;
    u8 H = (hl_ >> 8) & 0xFF;
    u8 L = hl_ & 0xFF;
    
    u16 log_pc = pc_ - 1;
    u8 mem0 = mmu_->read_memory_8(log_pc);
    u8 mem1 = mmu_->read_memory_8(log_pc + 1);
    u8 mem2 = mmu_->read_memory_8(log_pc + 2);
    u8 mem3 = mmu_->read_memory_8(log_pc + 3);
    
    Logger::log(A, F, B, C, D, E, H, L, sp_, log_pc, mem0, mem1, mem2, mem3);
}

u8 CPU::execute_next_instruction() {
    // TODO(cycle-accuracy): When halted, the CPU should advance 4 T-cycles at a time
    // until an interrupt wakes it. Currently we return 4 which is correct per call,
    // but the caller adds interrupt handling cycles on top. On real HW, the wake-up
    // from HALT has specific timing: the interrupt is serviced on the NEXT M-cycle
    // after the IF bit is set, and there's an additional 4 T-cycle delay.
    if (halted_) {
        return 4;
    }
    
    // Handle delayed EI (IME is set after the instruction following EI)
    bool ei_was_pending = ei_pending_;
    ei_pending_ = false;
    
    current_opcode_ = fetchOpcode();
    
    // TODO(bug): Using unordered_map with pattern matching is dangerous because iteration
    // order is non-deterministic. When multiple patterns match the same opcode (e.g. opcode
    // 0x76 matches both op_ld_r_r mask=0xC0/pat=0x40 AND op_halt mask=0xFF/pat=0x76),
    // the wrong handler may be called depending on hash bucket ordering. This WILL cause
    // subtle, platform-dependent bugs. Fix: use a 256-entry lookup table (u8 -> handler)
    // built at init time, where more-specific patterns override less-specific ones.
    // See also: instruction_decoder.cpp registerInstructions().
    auto handler = op_handlers_[current_opcode_];
    if (handler == nullptr) {
        std::cout << "Undefined opcode: " << std::hex << static_cast<int>(current_opcode_) << std::endl;
        throw std::runtime_error("Undefined opcode");
    }

    u8 cycles = (this->*handler)();
    
    if (ei_was_pending) {
        ime_ = true;
    }
    
    return cycles;
}

u8 CPU::handle_interrupts() {
    // Check if there's a pending interrupt (without clearing it)
    bool has_pending = interrupt_controller_->has_pending_interrupt();
    
    // Wake from HALT if any interrupt is pending (even if IME is disabled)
    if (halted_ && has_pending) {
        halted_ = false;
    }
    
    if (!ime_) {
        return 0;
    }
    
    // Now get and clear the interrupt if IME is enabled
    u16 addr = interrupt_controller_->get_address_of_highest_priority_interrupt();
    if (addr != INTERRUPT_HANDLER_NONE_ADDRESS) {
        push_to_stack(pc_);
        pc_ = addr;
        ime_ = false;
        return 20;
    }
    return 0;
}

u8 CPU::fetchOpcode() {
    // HALT bug: if triggered, don't increment PC for this fetch
    if (halt_bug_triggered_) {
        halt_bug_triggered_ = false;
        return mmu_->read_memory_8(pc_);  // Read without incrementing PC
    }
    return mmu_->read_memory_8(pc_++);
}

u8 CPU::cb_ins_handler() {
    log(__func__);
    current_opcode_ = fetchOpcode();
    
    auto handler = cb_handlers_[current_opcode_];
    if (handler == nullptr) {
        std::cout << "Undefined CB opcode: " << std::hex << static_cast<int>(current_opcode_) << std::endl;
        throw std::runtime_error("Undefined CB opcode");
    }
    
    return (this->*handler)();
}

u16 CPU::fetch_u16() {
    u8 low = fetchOpcode();   // Fetch low byte first
    u8 high = fetchOpcode();  // Then high byte
    return make_u16(low, high);
}

void CPU::push_to_stack(u16 value) {
    // TODO(cycle-accuracy): On real hardware PUSH is 4 M-cycles:
    // M1: internal delay (SP decremented)
    // M2: write high byte to --SP
    // M3: write low byte to --SP
    // M4: (part of the instruction that called push)
    // Each write should tick PPU/timer. The high byte is written FIRST at SP-1,
    // then low byte at SP-2. Current bulk approach won't reproduce effects where
    // games rely on mid-push PPU state (e.g., push during mode transitions).
    sp_ -= 2;
    mmu_->write_memory_8(sp_ + 1, value >> 8);
    mmu_->write_memory_8(sp_, value & 0xFF);
}

u16 CPU::pop_from_stack() {
    u16 value = mmu_->read_memory_8(sp_) | (mmu_->read_memory_8(sp_ + 1) << 8);
    sp_ += 2;
    return value;
}

// ============================================================================
// Opcode Parameter Decoding
// ============================================================================

u8 CPU::read_first_register_8_bit_parameter() const {
    return (current_opcode_ >> 3) & 0x07;
}

u8 CPU::read_second_register_8_bit_parameter() const {
    return current_opcode_ & 0x07;
}

u8 CPU::read_first_register_16_bit_parameter() const {
    return (current_opcode_ >> 4) & 0x03;
}

u8 CPU::read_second_register_16_bit_parameter() const {
    return (current_opcode_ >> 2) & 0x03;
}

u8 CPU::read_bit_argument() const {
    return (current_opcode_ >> 3) & 0x07;
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

u16 CPU::read_register_16_bit_memory(u8 reg_num) {
    switch (reg_num) {
        case 0: return bc_;
        case 1: return de_;
        case 2: return hl_++;
        case 3: return hl_--;
        default: throw std::runtime_error("Invalid register number");
    }
}

void CPU::write_register_16_bit_memory(u8 reg_num, u16 value) {
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

u8 CPU::op_ld_r_r() {
    log(__func__);
    u8 src = read_second_register_8_bit_parameter();
    u8 dst = read_first_register_8_bit_parameter();
    write_register_8_bit(dst, read_register_8_bit(src));
    return (src == HL_IND || dst == HL_IND) ? 8 : 4;
}

u8 CPU::op_ld_r_imm() {
    log(__func__);
    u8 dst = read_first_register_8_bit_parameter();
    write_register_8_bit(dst, fetchOpcode());
    return 8;
}

u8 CPU::op_ld_hl_ind_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u16 address = hl_;
    u8 value = read_register_8_bit(source_register);
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

u8 CPU::op_ld_hl_ind_imm() {
    log(__func__);
    u16 address = hl_;
    u8 value = fetchOpcode();
    mmu_->write_memory_8(address, value);
    return 12; // 12 cycles
}

u8 CPU::op_ld_a_bc_ind() {
    log(__func__);
    u16 address = bc_;
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

u8 CPU::op_ld_a_de_ind() {
    log(__func__);
    u16 address = de_;
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

u8 CPU::op_ld_bc_ind_a() {
    log(__func__);
    u16 address = bc_;
    u8 value = getA();
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

u8 CPU::op_ld_de_ind_a() {
    log(__func__);
    u16 address = de_;
    u8 value = getA();
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

u8 CPU::op_ld_a_imm_ind() {
    log(__func__);
    u16 address = fetch_u16();
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    return 16; // 16 cycles
}

u8 CPU::op_ld_imm_ind_a() {
    log(__func__);
    u16 address = fetch_u16();
    u8 value = getA();
    mmu_->write_memory_8(address, value);
    return 16; // 16 cycles
}

u8 CPU::op_ldh_a_c_ind() {
    log(__func__);
    u16 address = 0xFF00 + getC();
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    return 8; // 8 cycles
}

u8 CPU::op_ldh_c_ind_a() {
    log(__func__);
    u16 address = 0xFF00 + getC();
    u8 value = getA();
    mmu_->write_memory_8(address, value);
    return 8; // 8 cycles
}

u8 CPU::op_ldh_a_imm_ind() {
    log(__func__);
    u16 address = 0xFF00 + fetchOpcode();
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    return 12; // 12 cycles
}

u8 CPU::op_ldh_imm_ind_a() {
    log(__func__);
    u8 value = getA();
    u16 address = 0xFF00 + fetchOpcode();
    mmu_->write_memory_8(address, value);
    return 12; // 12 cycles
}

u8 CPU::op_ld_a_hl_ind_dec() {
    log(__func__);
    u16 address = hl_;
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    hl_--;
    return 8; // 8 cycles
}

u8 CPU::op_ld_hl_ind_dec_a() {
    log(__func__);
    u16 address = hl_;
    u8 value = getA();
    mmu_->write_memory_8(address, value);
    hl_--;
    return 8; // 8 cycles
}

u8 CPU::op_ld_a_hl_ind_inc() {
    log(__func__);
    u16 address = hl_;
    u8 value = mmu_->read_memory_8(address);
    setA(value);
    hl_++;
    return 8; // 8 cycles
}

u8 CPU::op_ld_hl_ind_inc_a() {
    log(__func__);
    u16 address = hl_;
    u8 value = getA();
    mmu_->write_memory_8(address, value);
    hl_++;
    return 8; // 8 cycles
}

// 16-bit load instructions
u8 CPU::op_ld_rr_imm() {
    log(__func__);
    u8 register_number = read_first_register_16_bit_parameter();
    u16 value = fetch_u16();
    write_register_16_bit(register_number, value);
    return 12; // 12 cycles
}

u8 CPU::op_ld_imm_ind_sp() {
    log(__func__);
    u16 address = fetch_u16();
    mmu_->write_memory_8(address, sp_ & 0xFF);
    mmu_->write_memory_8(address + 1, sp_ >> 8);
    return 20; // 20 cycles (5 M-cycles)
}

u8 CPU::op_ld_sp_hl() {
    log(__func__);
    sp_ = hl_;
    return 8; // 8 cycles
}

u8 CPU::op_push_rr() {
    log(__func__);
    u8 register_number = read_first_register_16_bit_parameter();
    push_to_stack(read_register_16_bit_stack(register_number));
    return 16;
}

u8 CPU::op_pop_rr() {
    log(__func__);
    u8 register_number = read_first_register_16_bit_parameter();
    write_register_16_bit_stack(register_number, pop_from_stack());
    return 12;
}

u8 CPU::op_ld_hl_sp_e() {
    log(__func__);
    s8 offset = static_cast<s8>(fetchOpcode());
    // Flags are based on unsigned addition of lower bytes
    bool h_bit = ((sp_ & 0x0F) + (offset & 0x0F)) > 0x0F;
    bool c_bit = ((sp_ & 0xFF) + (static_cast<u8>(offset))) > 0xFF;
    hl_ = sp_ + offset;
    setFlagZ(0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 12; // 12 cycles
}

// 8-bit arithmetic and logical instructions
u8 CPU::op_add_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) > 0xFF;
    setA(getA() + value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_add_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) > 0x0F;
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) > 0xFF;
    setA(getA() + value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_adc_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) + getFlagC() > 0xFF;
    setA(getA() + value + getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_adc_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) + (value & 0x0F) + getFlagC() > 0x0F;
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) + (static_cast<u16>(value) & 0xFF) + getFlagC() > 0xFF;
    setA(getA() + value + getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_sub_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
    setA(getA() - value);
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_sub_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
    setA(getA() - value);
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_sbc_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF) + getFlagC();
    setA(getA() - value - getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_sbc_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) < (value & 0x0F) + getFlagC();
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF) + getFlagC();
    setA(getA() - value - getFlagC());
    setFlagZ(getA() == 0);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_cp_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
    setFlagZ(getA() == value);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_cp_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    bool h_bit = (getA() & 0x0F) < (value & 0x0F);
    bool c_bit = (static_cast<u16>(getA()) & 0xFF) < (static_cast<u16>(value) & 0xFF);
    setFlagZ(getA() == value);
    setFlagN(1);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_inc_r() {
    log(__func__);
    u8 destination_register = read_first_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    value++;
    bool h_bit = (value & 0x0F) == 0x00;  // H set when lower nibble wraps from 0xF to 0x0
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(h_bit);
    return cycles_for_reg(destination_register, 4, 8); // (HL) = read+modify+write = 12 cycles
}

u8 CPU::op_dec_r() {
    log(__func__);
    u8 destination_register = read_first_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    value--;
    bool h_bit = (value & 0x0F) == 0x0F;  // H set when lower nibble wraps from 0x0 to 0xF (borrow)
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(1);
    setFlagH(h_bit);
    return cycles_for_reg(destination_register, 4, 8); // (HL) = read+modify+write = 12 cycles
}

u8 CPU::op_and_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_and_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    setA(getA() & value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(1);
    setFlagC(0);
    return 8; // 8 cycles
}

u8 CPU::op_or_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_or_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    setA(getA() | value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

u8 CPU::op_xor_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return cycles_for_reg(source_register, 4, 4);
}

u8 CPU::op_xor_imm() {
    log(__func__);
    u8 value = fetchOpcode();
    setA(getA() ^ value);
    setFlagZ(getA() == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

u8 CPU::op_ccf() {
    log(__func__);
    setFlagC(!getFlagC());
    setFlagN(0);
    setFlagH(0);
    return 4; // 4 cycles
}

u8 CPU::op_scf() {
    log(__func__);
    setFlagC(1);
    setFlagN(0);
    setFlagH(0);
    return 4; // 4 cycles
}

u8 CPU::op_daa() {
    log(__func__);
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
    setFlagH(0);  // H is always reset
    setFlagC(should_set_carry);
    return 4; // 4 cycles
}

u8 CPU::op_cpl() {
    log(__func__);
    setA(~getA());
    setFlagN(1);
    setFlagH(1);
    return 4; // 4 cycles
}

// 16-bit arithmetic instructions
u8 CPU::op_inc_rr() {
    log(__func__);
    u8 register_number = read_first_register_16_bit_parameter();
    u16 value = read_register_16_bit(register_number);
    value++;
    write_register_16_bit(register_number, value);
    return 8; // 8 cycles
}

u8 CPU::op_dec_rr() {
    log(__func__);
    u8 register_number = read_first_register_16_bit_parameter();
    u16 value = read_register_16_bit(register_number);
    value--;
    write_register_16_bit(register_number, value);
    return 8; // 8 cycles
}

u8 CPU::op_add_hl_rr() {
    log(__func__);
    u8 source_register = read_first_register_16_bit_parameter();
    u16 value = read_register_16_bit(source_register);
    // For 16-bit ADD, H is carry from bit 11, C is carry from bit 15
    bool h_bit = (hl_ & 0x0FFF) + (value & 0x0FFF) > 0x0FFF;
    bool c_bit = (static_cast<u32>(hl_) + value) > 0xFFFF;
    hl_ = hl_ + value;
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_add_sp_e() {
    log(__func__);
    s8 offset = static_cast<s8>(fetchOpcode());
    // Flags are based on unsigned addition of lower bytes
    bool h_bit = ((sp_ & 0x0F) + (offset & 0x0F)) > 0x0F;
    bool c_bit = ((sp_ & 0xFF) + (static_cast<u8>(offset))) > 0xFF;
    sp_ = sp_ + offset;
    setFlagZ(0);
    setFlagN(0);
    setFlagH(h_bit);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

// Rotate, shift, and bit operation instructions
u8 CPU::op_rlca() {
    log(__func__);
    u8 value = getA();
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

u8 CPU::op_rrca() {
    log(__func__);
    u8 value = getA();
    bool c_bit = value & 0x01;
    value = (value >> 1) | (c_bit << 7);
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

u8 CPU::op_rla() {
    log(__func__);
    u8 value = getA();
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    setA(value);
    setFlagZ(0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 4; // 4 cycles
}

u8 CPU::op_rra() {
    log(__func__);
    u8 value = getA();
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
u8 CPU::op_rlc_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_rlc_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value >> 7;
    value = (value << 1) | c_bit;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_rrc_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (c_bit << 7);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_rrc_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (c_bit << 7);
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_rl_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_rl_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value >> 7;
    value = (value << 1) | getFlagC();
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_rr_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (getFlagC() << 7);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_rr_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (getFlagC() << 7);
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_sla_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value >> 7;
    value = value << 1;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_sla_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value >> 7;
    value = value << 1;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_sra_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (value & 0x80);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_sra_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = (value >> 1) | (value & 0x80);  // SRA preserves bit 7 (sign)
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_swap_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    value = (value << 4) | (value >> 4);
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 8; // 8 cycles
}

u8 CPU::op_swap_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    value = (value << 4) | (value >> 4);
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(0);
    return 16; // 16 cycles
}

u8 CPU::op_srl_r() {
    log(__func__);
    u8 destination_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(destination_register);
    bool c_bit = value & 0x01;
    value = value >> 1;
    write_register_8_bit(destination_register, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 8; // 8 cycles
}

u8 CPU::op_srl_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    bool c_bit = value & 0x01;
    value = value >> 1;
    mmu_->write_memory_8(hl_, value);
    setFlagZ(value == 0);
    setFlagN(0);
    setFlagH(0);
    setFlagC(c_bit);
    return 16; // 16 cycles
}

u8 CPU::op_bit_b_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    setFlagZ(!test_bit(value, read_bit_argument()));
    setFlagN(false);
    setFlagH(true);
    return 8; // 8 cycles
}

u8 CPU::op_bit_b_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    setFlagZ(!test_bit(value, read_bit_argument()));
    setFlagN(false);
    setFlagH(true);
    return 12; // 12 cycles
}

u8 CPU::op_res_b_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    value = clear_bit(value, read_bit_argument());
    write_register_8_bit(source_register, value);
    return 8; // 8 cycles
}

u8 CPU::op_res_b_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    value = clear_bit(value, read_bit_argument());
    mmu_->write_memory_8(hl_, value);
    return 16; // 16 cycles
}

u8 CPU::op_set_b_r() {
    log(__func__);
    u8 source_register = read_second_register_8_bit_parameter();
    u8 value = read_register_8_bit(source_register);
    value = set_bit(value, read_bit_argument(), true);
    write_register_8_bit(source_register, value);
    return 8; // 8 cycles
}

u8 CPU::op_set_b_hl_ind() {
    log(__func__);
    u8 value = mmu_->read_memory_8(hl_);
    value = set_bit(value, read_bit_argument(), true);
    mmu_->write_memory_8(hl_, value);
    return 16; // 16 cycles
}


// Control flow instructions
u8 CPU::op_jp_imm() {
    log(__func__);
    u16 address = fetch_u16();
    pc_ = address;
    return 16; // 16 cycles
}

u8 CPU::op_jp_hl() {
    log(__func__);
    pc_ = hl_;
    return 4; // 4 cycles
}

u8 CPU::op_jp_cc_imm() {
    log(__func__);
    u16 address = fetch_u16();
    if (read_condition_argument()) {
        pc_ = address;
        return 16; // 16 cycles
    }
    return 12; // 12 cycles
}

u8 CPU::op_jr_e() {
    log(__func__);
    u8 value = fetchOpcode();
    if (value >> 7) {
        value = ~value + 1;
        pc_ = pc_ - value;
    } else {
        pc_ = pc_ + value;
    }
    return 12; // 12 cycles
}

u8 CPU::op_jr_cc_e() {
    log(__func__);
    u8 value = fetchOpcode();
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

u8 CPU::op_call_imm() {
    log(__func__);
    u16 address = fetch_u16();
    push_to_stack(pc_);
    pc_ = address;
    return 24;
}

u8 CPU::op_call_cc_imm() {
    log(__func__);
    u16 address = fetch_u16();
    if (read_condition_argument()) {
        push_to_stack(pc_);
        pc_ = address;
        return 24;
    }
    return 12;
}

u8 CPU::op_ret() {
    log(__func__);
    pc_ = pop_from_stack();
    return 16;
}

u8 CPU::op_ret_cc() {
    log(__func__);
    if (read_condition_argument()) {
        pc_ = pop_from_stack();
        return 20;
    }
    return 8;
}

u8 CPU::op_reti() {
    log(__func__);
    pc_ = pop_from_stack();
    ime_ = true;
    return 16;
}

u8 CPU::op_rst_imm() {
    log(__func__);
    push_to_stack(pc_);
    pc_ = read_bit_argument() << 3;
    return 16;
}

// Miscellaneous instructions
u8 CPU::op_halt() {
    log(__func__);
    // TODO(cycle-accuracy): The HALT bug has additional edge cases not handled here:
    // 1. If IME=1 and there's a pending interrupt, CPU doesn't halt - it immediately
    //    services the interrupt (the HALT itself consumes 4 cycles then ISR runs).
    // 2. If IME=0 and IE&IF!=0, the HALT bug triggers: the byte after HALT is read
    //    twice (PC fails to increment). But the exact behavior differs based on
    //    whether the next instruction is a multi-byte one (can corrupt execution).
    // 3. There's a 4-cycle delay after HALT before the CPU actually stops.
    if (!ime_ && interrupt_controller_->has_pending_interrupt()) {
        halt_bug_triggered_ = true;
    } else {
        halted_ = true;
    }
    return 4;
}

u8 CPU::op_stop() {
    log(__func__);
    // TODO: STOP is a 2-byte instruction (0x10 0x00) but we only consume the prefix.
    // The second byte (0x00) should be fetched and discarded. Without this, the 0x00
    // is left in the stream and treated as a NOP, which happens to work by accident
    // but is incorrect. Also, STOP should halt CPU & LCD until a button is pressed
    // (on CGB it triggers a speed switch if the KEY1 register is prepared).
    mmu_->write_memory_8(TIMER_REGISTER_DIV, 0x00);
    return 4;
}

u8 CPU::op_di() {
    log(__func__);
    ime_ = false;
    return 4;
}

u8 CPU::op_ei() {
    log(__func__);
    ei_pending_ = true;  // IME will be set after the next instruction
    return 4; // 4 cycles
}

u8 CPU::op_nop() {
    log(__func__);
    return 4; // 4 cycles
}