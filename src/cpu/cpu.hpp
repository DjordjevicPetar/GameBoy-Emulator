#pragma once

#include "../utils/types.hpp"

#include "../utils/constants.hpp"
#include <array>

// Forward declarations
class MMU;
class InterruptController;

class CPU {
    using Handler = void (CPU::*)();

public:
    CPU(MMU* mmu, InterruptController* interrupt_controller);
    
    void process_cycle();
    
    bool getIME() const { return ime_; }
    void setIME(bool value) { ime_ = value; }
    bool isStopped() const { return stopped_; }
    void wake() { stopped_ = false; }

private:

    enum class State_Machine {
        FETCH_NEXT_INSTRUCTION,
        INITIAL_PHASE,
        ASSIGN_VALUE_TO_REGISTER,
        ASSIGN_VALUE_TO_REGISTER_LOW_BYTE,
        ASSIGN_VALUE_TO_REGISTER_HIGH_BYTE,
        WRITE_VALUE_TO_MEMORY,
        WRITE_VALUE_TO_MEMORY_LOW_BYTE,
        WRITE_VALUE_TO_MEMORY_HIGH_BYTE,
        READ_MEMORY,
        READ_MEMORY_LOW_BYTE,
        READ_MEMORY_HIGH_BYTE,
        DECREMENT_SP,
        PROCESS_INTERRUPTS,
        NO_OP,
    };
    // Dependencies
    MMU* mmu_;
    InterruptController* interrupt_controller_;
    
    // State
    bool ime_ = false;           // Interrupt Master Enable
    bool ei_pending_ = false;    // EI delay (IME enabled after next instruction)
    bool ei_was_pending_ = false;
    bool halted_ = false;        // CPU halted, waiting for interrupt
    bool halt_bug_triggered_ = false;  // HALT bug: next instruction read doesn't increment PC
    bool stopped_ = false;            // CPU stopped, waiting for button press
    u16 memory_temp_ = 0;
    State_Machine state_machine_ = State_Machine::FETCH_NEXT_INSTRUCTION;

    // Interrupt handling
    bool process_interrupts_ = false;

    // Registers
    u16 af_ = 0;
    u16 bc_ = 0;
    u16 de_ = 0;
    u16 hl_ = 0;
    u16 sp_ = 0;
    u16 pc_ = PROGRAM_COUNTER_START;

    std::array<Handler, 256> op_handlers_{};
    std::array<Handler, 256> cb_handlers_{};

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
    u8 read_condition_argument() const;

    // Register read/write by number
    u8 read_register_8_bit(u8 reg_num) const;
    void write_register_8_bit(u8 reg_num, u8 value);
    u16 read_register_16_bit(u8 reg_num) const;
    void write_register_16_bit(u8 reg_num, u16 value);
    u16 read_register_16_bit_stack(u8 reg_num) const;
    void write_register_16_bit_stack(u8 reg_num, u16 value);
    // Utility
    void log_state();
    void read_memory_8(u16 address, bool high_byte = false);
    void write_memory_8(u16 address, u8 value);
    
    // Interrupt handling (called at instruction boundaries inside process_cycle)
    void process_interrupts();
    void detect_interrupts();

    // CB prefix handler
    void cb_ins_handler();

    // CPU_Decoder functions
    void registerInstructions();
    void registerCbInstructions();
    void add_instruction(std::array<Handler, 256>& handlers, u8 mask, u8 pattern, Handler instruction);
    void fetch_next_instruction(std::array<Handler, 256>& handlers);
    Handler current_instruction_handler_;
    u8 current_opcode_ = 0;
    
    // 8-bit load instructions
    void op_ld_r_r();
    void op_ld_r_imm();
    void op_ld_r_hl_ind();
    void op_ld_hl_ind_r();
    void op_ld_hl_ind_imm();
    void op_ld_a_bc_ind();
    void op_ld_a_de_ind();
    void op_ld_bc_ind_a();
    void op_ld_de_ind_a();
    void op_ld_a_imm_ind();
    void op_ld_imm_ind_a();
    void op_ldh_a_c_ind();
    void op_ldh_c_ind_a();
    void op_ldh_a_imm_ind();
    void op_ldh_imm_ind_a();
    void op_ld_a_hl_ind_dec();
    void op_ld_hl_ind_dec_a();
    void op_ld_a_hl_ind_inc();
    void op_ld_hl_ind_inc_a();
    
    // 16-bit load instructions
    void op_ld_rr_imm();
    void op_ld_imm_ind_sp();
    void op_ld_sp_hl();
    void op_push_rr();
    void op_pop_rr();
    void op_ld_hl_sp_e();
    
    // 8-bit arithmetic/logic
    void op_add_r();
    void op_add_hl_ind();
    void op_add_imm();
    void op_adc_r();
    void op_adc_hl_ind();
    void op_adc_imm();
    void op_sub_r();
    void op_sub_hl_ind();
    void op_sub_imm();
    void op_sbc_r();
    void op_sbc_hl_ind();
    void op_sbc_imm();
    void op_cp_r();
    void op_cp_hl_ind();
    void op_cp_imm();
    void op_inc_r();
    void op_inc_hl_ind();
    void op_dec_r();
    void op_dec_hl_ind();
    void op_and_r();
    void op_and_hl_ind();
    void op_and_imm();
    void op_or_r();
    void op_or_hl_ind();
    void op_or_imm();
    void op_xor_r();
    void op_xor_hl_ind();
    void op_xor_imm();
    void op_ccf();
    void op_scf();
    void op_daa();
    void op_cpl();
    
    // 16-bit arithmetic
    void op_inc_rr();
    void op_dec_rr();
    void op_add_hl_rr();
    void op_add_sp_e();
    
    // Rotate/shift (non-CB)
    void op_rlca();
    void op_rrca();
    void op_rla();
    void op_rra();

    // CB prefix instructions
    void op_rlc_r();
    void op_rlc_hl_ind();
    void op_rrc_r();
    void op_rrc_hl_ind();
    void op_rl_r();
    void op_rl_hl_ind();
    void op_rr_r();
    void op_rr_hl_ind();
    void op_sla_r();
    void op_sla_hl_ind();
    void op_sra_r();
    void op_sra_hl_ind();
    void op_swap_r();
    void op_swap_hl_ind();
    void op_srl_r();
    void op_srl_hl_ind();
    void op_bit_b_r();
    void op_bit_b_hl_ind();
    void op_res_b_r();
    void op_res_b_hl_ind();
    void op_set_b_r();
    void op_set_b_hl_ind();
    
    // Control flow
    void op_jp_imm();
    void op_jp_hl();
    void op_jp_cc_imm();
    void op_jr_e();
    void op_jr_cc_e();
    void op_call_imm();
    void op_call_cc_imm();
    void op_ret();
    void op_ret_cc();
    void op_reti();
    void op_rst_imm();
    
    // Miscellaneous
    void op_halt();
    void op_stop();
    void op_di();
    void op_ei();
    void op_nop();
};
