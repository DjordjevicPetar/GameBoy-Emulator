#pragma once

#include "utils/types.hpp"


#include "utils/constants.hpp"
#include "instruction_decoder.hpp"
#include <string>
#include <unordered_map>

// Forward declarations
class MMU;
class InterruptController;

class CPU {
    friend class InstructionDecoder;

public:
    CPU(MMU* mmu, InterruptController* interrupt_controller);
    
    u8 execute_next_instruction();
    u8 handle_interrupts();
    
    bool getIME() const { return ime_; }
    void setIME(bool value) { ime_ = value; }

private:
    // Dependencies
    MMU* mmu_;
    InterruptController* interrupt_controller_;
    
    // State
    u8 current_opcode_ = 0;
    bool ime_ = false;           // Interrupt Master Enable
    bool ei_pending_ = false;    // EI delay (IME enabled after next instruction)
    bool halted_ = false;        // CPU halted, waiting for interrupt
    bool halt_bug_triggered_ = false;  // HALT bug: next instruction read doesn't increment PC

    // Cycle helpers - register 6 is (HL) which requires memory access
    static constexpr u8 HL_IND = 6;
    static constexpr u8 cycles_for_reg(u8 reg, u8 base, u8 hl_extra) {
        return (reg == HL_IND) ? (base + hl_extra) : base;
    }

    // Registers
    u16 af_ = 0;
    u16 bc_ = 0;
    u16 de_ = 0;
    u16 hl_ = 0;
    u16 sp_ = 0;
    u16 pc_ = PROGRAM_COUNTER_START;

    // Instruction handlers
    using HandlerMap = std::unordered_map<InstructionDecoder::Op, u8 (CPU::*)(), InstructionDecoder::OpHash>;
    HandlerMap op_handlers_;
    HandlerMap cb_handlers_;

    // Register access helpers - 8-bit
    u8 getA() const { return (af_ >> 8) & 0xFF; }
    u8 getF() const { return af_ & 0xF0; }
    u8 getB() const { return (bc_ >> 8) & 0xFF; }
    u8 getC() const { return bc_ & 0xFF; }
    u8 getD() const { return (de_ >> 8) & 0xFF; }
    u8 getE() const { return de_ & 0xFF; }
    u8 getH() const { return (hl_ >> 8) & 0xFF; }
    u8 getL() const { return hl_ & 0xFF; }
    
    void setA(u8 value) { af_ = (af_ & 0x00F0) | (value << 8); }
    void setF(u8 value) { af_ = (af_ & 0xFF00) | (value & 0xF0); }
    void setB(u8 value) { bc_ = (bc_ & 0x00FF) | (value << 8); }
    void setC(u8 value) { bc_ = (bc_ & 0xFF00) | value; }
    void setD(u8 value) { de_ = (de_ & 0x00FF) | (value << 8); }
    void setE(u8 value) { de_ = (de_ & 0xFF00) | value; }
    void setH(u8 value) { hl_ = (hl_ & 0x00FF) | (value << 8); }
    void setL(u8 value) { hl_ = (hl_ & 0xFF00) | value; }

    // Flag access helpers
    bool getFlagZ() const { return af_ & 0x80; }
    bool getFlagN() const { return af_ & 0x40; }
    bool getFlagH() const { return af_ & 0x20; }
    bool getFlagC() const { return af_ & 0x10; }
    void setFlagZ(bool value) { af_ = (af_ & 0xFF7F) | (value ? 0x80 : 0x00); }
    void setFlagN(bool value) { af_ = (af_ & 0xFFBF) | (value ? 0x40 : 0x00); }
    void setFlagH(bool value) { af_ = (af_ & 0xFFDF) | (value ? 0x20 : 0x00); }
    void setFlagC(bool value) { af_ = (af_ & 0xFFEF) | (value ? 0x10 : 0x00); }

    // Opcode parameter decoding
    u8 read_first_register_8_bit_parameter() const;
    u8 read_second_register_8_bit_parameter() const;
    u8 read_first_register_16_bit_parameter() const;
    u8 read_second_register_16_bit_parameter() const;
    u8 read_bit_argument() const;
    u8 read_condition_argument() const;

    // Register read/write by number
    u8 read_register_8_bit(u8 reg_num) const;
    void write_register_8_bit(u8 reg_num, u8 value);
    u16 read_register_16_bit(u8 reg_num) const;
    void write_register_16_bit(u8 reg_num, u16 value);
    u16 read_register_16_bit_stack(u8 reg_num) const;
    void write_register_16_bit_stack(u8 reg_num, u16 value);
    u16 read_register_16_bit_memory(u8 reg_num);
    void write_register_16_bit_memory(u8 reg_num, u16 value);

    // Utility
    u8 fetchOpcode();
    u16 fetch_u16();  // Fetch 16-bit value (low byte first, then high byte)
    static u16 make_u16(u8 low, u8 high) { return (static_cast<u16>(high) << 8) | low; }
    void log(const std::string& func_name, const std::string& details = "");
    
    // Stack operations
    void push_to_stack(u16 value);
    u16 pop_from_stack();
    
    // CB prefix handler
    u8 cb_ins_handler();
    
    // 8-bit load instructions
    u8 op_ld_r_r();       // handles LD r,r' including LD r,(HL)
    u8 op_ld_r_imm();
    u8 op_ld_hl_ind_r();
    u8 op_ld_hl_ind_imm();
    u8 op_ld_a_bc_ind();
    u8 op_ld_a_de_ind();
    u8 op_ld_bc_ind_a();
    u8 op_ld_de_ind_a();
    u8 op_ld_a_imm_ind();
    u8 op_ld_imm_ind_a();
    u8 op_ldh_a_c_ind();
    u8 op_ldh_c_ind_a();
    u8 op_ldh_a_imm_ind();
    u8 op_ldh_imm_ind_a();
    u8 op_ld_a_hl_ind_dec();
    u8 op_ld_hl_ind_dec_a();
    u8 op_ld_a_hl_ind_inc();
    u8 op_ld_hl_ind_inc_a();
    
    // 16-bit load instructions
    u8 op_ld_rr_imm();
    u8 op_ld_imm_ind_sp();
    u8 op_ld_sp_hl();
    u8 op_push_rr();
    u8 op_pop_rr();
    u8 op_ld_hl_sp_e();
    
    // 8-bit arithmetic/logic - _r handlers include (HL) with proper cycle count
    u8 op_add_r();
    u8 op_add_imm();
    u8 op_adc_r();
    u8 op_adc_imm();
    u8 op_sub_r();
    u8 op_sub_imm();
    u8 op_sbc_r();
    u8 op_sbc_imm();
    u8 op_cp_r();
    u8 op_cp_imm();
    u8 op_inc_r();
    u8 op_dec_r();
    u8 op_and_r();
    u8 op_and_imm();
    u8 op_or_r();
    u8 op_or_imm();
    u8 op_xor_r();
    u8 op_xor_imm();
    u8 op_ccf();
    u8 op_scf();
    u8 op_daa();
    u8 op_cpl();
    
    // 16-bit arithmetic
    u8 op_inc_rr();
    u8 op_dec_rr();
    u8 op_add_hl_rr();
    u8 op_add_sp_e();
    
    // Rotate/shift (non-CB)
    u8 op_rlca();
    u8 op_rrca();
    u8 op_rla();
    u8 op_rra();

    // CB prefix instructions
    u8 op_rlc_r();
    u8 op_rlc_hl_ind();
    u8 op_rrc_r();
    u8 op_rrc_hl_ind();
    u8 op_rl_r();
    u8 op_rl_hl_ind();
    u8 op_rr_r();
    u8 op_rr_hl_ind();
    u8 op_sla_r();
    u8 op_sla_hl_ind();
    u8 op_sra_r();
    u8 op_sra_hl_ind();
    u8 op_swap_r();
    u8 op_swap_hl_ind();
    u8 op_srl_r();
    u8 op_srl_hl_ind();
    u8 op_bit_b_r();
    u8 op_bit_b_hl_ind();
    u8 op_res_b_r();
    u8 op_res_b_hl_ind();
    u8 op_set_b_r();
    u8 op_set_b_hl_ind();
    
    // Control flow
    u8 op_jp_imm();
    u8 op_jp_hl();
    u8 op_jp_cc_imm();
    u8 op_jr_e();
    u8 op_jr_cc_e();
    u8 op_call_imm();
    u8 op_call_cc_imm();
    u8 op_ret();
    u8 op_ret_cc();
    u8 op_reti();
    u8 op_rst_imm();
    
    // Miscellaneous
    u8 op_halt();
    u8 op_stop();
    u8 op_di();
    u8 op_ei();
    u8 op_nop();
};

