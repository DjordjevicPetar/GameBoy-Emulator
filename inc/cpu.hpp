#ifndef CPU_HPP_
#define CPU_HPP_

#include "constants.hpp"
#include "instruction_decoder.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>

// Forward declarations
class MMU;
class InterruptController;

class CPU {
    friend class InstructionDecoder;

public:
    CPU(MMU* mmu, InterruptController* interrupt_controller);
    
    uint8_t execute_next_instruction();
    uint8_t handle_interrupts();
    
    bool getIME() const { return ime_; }
    void setIME(bool value) { ime_ = value; }

private:
    // Dependencies
    MMU* mmu_;
    InterruptController* interrupt_controller_;
    
    // State
    uint8_t current_opcode_ = 0;
    bool ime_ = false;  // Interrupt Master Enable

    // Registers
    uint16_t af_ = 0;
    uint16_t bc_ = 0;
    uint16_t de_ = 0;
    uint16_t hl_ = 0;
    uint16_t sp_ = 0;
    uint16_t pc_ = PROGRAM_COUNTER_START;

    // Instruction handlers
    using HandlerMap = std::unordered_map<InstructionDecoder::Op, uint8_t (CPU::*)(), InstructionDecoder::OpHash>;
    HandlerMap op_handlers_;
    HandlerMap cb_handlers_;

    // Register access helpers - 8-bit
    uint8_t getA() const { return (af_ >> 8) & 0xFF; }
    uint8_t getF() const { return af_ & 0xF0; }
    uint8_t getB() const { return (bc_ >> 8) & 0xFF; }
    uint8_t getC() const { return bc_ & 0xFF; }
    uint8_t getD() const { return (de_ >> 8) & 0xFF; }
    uint8_t getE() const { return de_ & 0xFF; }
    uint8_t getH() const { return (hl_ >> 8) & 0xFF; }
    uint8_t getL() const { return hl_ & 0xFF; }
    
    void setA(uint8_t value) { af_ = (af_ & 0x00F0) | (value << 8); }
    void setF(uint8_t value) { af_ = (af_ & 0xFF00) | (value & 0xF0); }
    void setB(uint8_t value) { bc_ = (bc_ & 0x00FF) | (value << 8); }
    void setC(uint8_t value) { bc_ = (bc_ & 0xFF00) | value; }
    void setD(uint8_t value) { de_ = (de_ & 0x00FF) | (value << 8); }
    void setE(uint8_t value) { de_ = (de_ & 0xFF00) | value; }
    void setH(uint8_t value) { hl_ = (hl_ & 0x00FF) | (value << 8); }
    void setL(uint8_t value) { hl_ = (hl_ & 0xFF00) | value; }

    // Flag access helpers
    uint8_t getFlagZ() const { return af_ & 0x80; }
    uint8_t getFlagN() const { return af_ & 0x40; }
    uint8_t getFlagH() const { return af_ & 0x20; }
    uint8_t getFlagC() const { return af_ & 0x10; }
    void setFlagZ(uint8_t value) { af_ = (af_ & 0xFF7F) | (value << 7); }
    void setFlagN(uint8_t value) { af_ = (af_ & 0xFFBF) | (value << 6); }
    void setFlagH(uint8_t value) { af_ = (af_ & 0xFFDF) | (value << 5); }
    void setFlagC(uint8_t value) { af_ = (af_ & 0xFFEF) | (value << 4); }

    // Opcode parameter decoding
    uint8_t read_first_register_8_bit_parameter() const;
    uint8_t read_second_register_8_bit_parameter() const;
    uint8_t read_first_register_16_bit_parameter() const;
    uint8_t read_second_register_16_bit_parameter() const;
    uint8_t read_bit_argument() const;
    uint8_t read_condition_argument() const;

    // Register read/write by number
    uint8_t read_register_8_bit(uint8_t reg_num) const;
    void write_register_8_bit(uint8_t reg_num, uint8_t value);
    uint16_t read_register_16_bit(uint8_t reg_num) const;
    void write_register_16_bit(uint8_t reg_num, uint16_t value);
    uint16_t read_register_16_bit_stack(uint8_t reg_num) const;
    void write_register_16_bit_stack(uint8_t reg_num, uint16_t value);
    uint16_t read_register_16_bit_memory(uint8_t reg_num);
    void write_register_16_bit_memory(uint8_t reg_num, uint16_t value);

    // Utility
    uint8_t fetchOpcode();
    uint16_t endian_swap(uint8_t low, uint8_t high) const;
    void log(const std::string& func_name, const std::string& details = "");
    
    // Stack operations
    void push_to_stack(uint16_t value);
    uint16_t pop_from_stack();
    
    // CB prefix handler
    uint8_t cb_ins_handler();
    
    // 8-bit load instructions
    uint8_t op_ld_r_r();
    uint8_t op_ld_r_imm();
    uint8_t op_ld_r_hl_ind();
    uint8_t op_ld_hl_ind_r();
    uint8_t op_ld_hl_ind_imm();
    uint8_t op_ld_a_bc_ind();
    uint8_t op_ld_a_de_ind();
    uint8_t op_ld_bc_ind_a();
    uint8_t op_ld_de_ind_a();
    uint8_t op_ld_a_imm_ind();
    uint8_t op_ld_imm_ind_a();
    uint8_t op_ldh_a_c_ind();
    uint8_t op_ldh_c_ind_a();
    uint8_t op_ldh_a_imm_ind();
    uint8_t op_ldh_imm_ind_a();
    uint8_t op_ld_a_hl_ind_dec();
    uint8_t op_ld_hl_ind_dec_a();
    uint8_t op_ld_a_hl_ind_inc();
    uint8_t op_ld_hl_ind_inc_a();
    
    // 16-bit load instructions
    uint8_t op_ld_rr_imm();
    uint8_t op_ld_imm_ind_sp();
    uint8_t op_ld_sp_hl();
    uint8_t op_push_rr();
    uint8_t op_pop_rr();
    uint8_t op_ld_hl_sp_e();
    
    // 8-bit arithmetic/logic
    uint8_t op_add_r();
    uint8_t op_add_hl_ind();
    uint8_t op_add_imm();
    uint8_t op_adc_r();
    uint8_t op_adc_hl_ind();
    uint8_t op_adc_imm();
    uint8_t op_sub_r();
    uint8_t op_sub_hl_ind();
    uint8_t op_sub_imm();
    uint8_t op_sbc_r();
    uint8_t op_sbc_hl_ind();
    uint8_t op_sbc_imm();
    uint8_t op_cp_r();
    uint8_t op_cp_hl_ind();
    uint8_t op_cp_imm();
    uint8_t op_inc_r();
    uint8_t op_inc_hl_ind();
    uint8_t op_dec_r();
    uint8_t op_dec_hl_ind();
    uint8_t op_and_r();
    uint8_t op_and_hl_ind();
    uint8_t op_and_imm();
    uint8_t op_or_r();
    uint8_t op_or_hl_ind();
    uint8_t op_or_imm();
    uint8_t op_xor_r();
    uint8_t op_xor_hl_ind();
    uint8_t op_xor_imm();
    uint8_t op_ccf();
    uint8_t op_scf();
    uint8_t op_daa();
    uint8_t op_cpl();
    
    // 16-bit arithmetic
    uint8_t op_inc_rr();
    uint8_t op_dec_rr();
    uint8_t op_add_hl_rr();
    uint8_t op_add_sp_e();
    
    // Rotate/shift (non-CB)
    uint8_t op_rlca();
    uint8_t op_rrca();
    uint8_t op_rla();
    uint8_t op_rra();

    // CB prefix instructions
    uint8_t op_rlc_r();
    uint8_t op_rlc_hl_ind();
    uint8_t op_rrc_r();
    uint8_t op_rrc_hl_ind();
    uint8_t op_rl_r();
    uint8_t op_rl_hl_ind();
    uint8_t op_rr_r();
    uint8_t op_rr_hl_ind();
    uint8_t op_sla_r();
    uint8_t op_sla_hl_ind();
    uint8_t op_sra_r();
    uint8_t op_sra_hl_ind();
    uint8_t op_swap_r();
    uint8_t op_swap_hl_ind();
    uint8_t op_srl_r();
    uint8_t op_srl_hl_ind();
    uint8_t op_bit_b_r();
    uint8_t op_bit_b_hl_ind();
    uint8_t op_res_b_r();
    uint8_t op_res_b_hl_ind();
    uint8_t op_set_b_r();
    uint8_t op_set_b_hl_ind();
    
    // Control flow
    uint8_t op_jp_imm();
    uint8_t op_jp_hl();
    uint8_t op_jp_cc_imm();
    uint8_t op_jr_e();
    uint8_t op_jr_cc_e();
    uint8_t op_call_imm();
    uint8_t op_call_cc_imm();
    uint8_t op_ret();
    uint8_t op_ret_cc();
    uint8_t op_reti();
    uint8_t op_rst_imm();
    
    // Miscellaneous
    uint8_t op_halt();
    uint8_t op_stop();
    uint8_t op_di();
    uint8_t op_ei();
    uint8_t op_nop();
};

#endif
