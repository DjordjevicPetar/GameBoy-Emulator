#ifndef EMULATOR_H_
#define EMULATOR_H_

#include "../inc/constants.hpp"
#include "../inc/instruction_decoder.hpp"
#include "../inc/mmu.hpp"
#include <unordered_map>
#include <string>
#include <stdexcept>

// Forward declaration
class InstructionDecoder;

class GameBoyEmulator {
    // Allow InstructionDecoder to access private members
    friend class InstructionDecoder;

private:

    uint8_t current_opcode;
    uint32_t cycles_executed;

    bool IME;

    // Memory
    MMU mmu_instance;
    uint16_t endian_swap(uint8_t value1, uint8_t value2) const;

    // Stop conditions
    bool stop_cpu;
    bool stop_gpu;

    // Registers
    uint16_t AF;    
    uint16_t BC;
    uint16_t DE;
    uint16_t HL;
    uint16_t SP;
    uint16_t PC = PROGRAM_COUNTER_START;

    // register 8-bit arguments helper functions
    inline uint8_t read_first_register_8_bit_parameter() const;
    inline uint8_t read_second_register_8_bit_parameter() const;
    // register 16-bit arguments helper functions
    inline uint8_t read_first_register_16_bit_parameter() const;
    inline uint8_t read_second_register_16_bit_parameter() const;
    // r8 arguments helper functions
    uint8_t read_register_8_bit(uint8_t register_number) const;
    void write_register_8_bit(uint8_t register_number, uint8_t value);
    // r16 arguments helper functions
    uint16_t read_register_16_bit(uint8_t register_number) const;
    void write_register_16_bit(uint8_t register_number, uint16_t value);
    // stack arguments helper functions
    uint16_t read_register_16_bit_stack(uint8_t register_number) const;
    void write_register_16_bit_stack(uint8_t register_number, uint16_t value);
    // memory arguments helper functions
    uint16_t read_register_16_bit_memory(uint8_t register_number);
    void write_register_16_bit_memory(uint8_t register_number, uint16_t value);
    // bit arguments helper functions
    uint8_t read_bit_argument() const;
    // condition arguments helper functions
    uint8_t read_condition_argument() const;

    // 8-bit registers helper functions
    inline uint8_t getA() const { return (AF >> 8) & 0xFF; }
    inline uint8_t getF() const { return AF & 0xF0; }
    inline uint8_t getB() const { return (BC >> 8) & 0xFF; }
    inline uint8_t getC() const { return BC & 0xFF; }
    inline uint8_t getD() const { return (DE >> 8) & 0xFF; }
    inline uint8_t getE() const { return DE & 0xFF; }
    inline uint8_t getH() const { return (HL >> 8) & 0xFF; }
    inline uint8_t getL() const { return HL & 0xFF; }
    
    inline void setA(uint8_t value) { AF = (AF & 0x00F0) | (value << 8); }
    inline void setF(uint8_t value) { AF = (AF & 0xFF00) | (value & 0xF0); }
    inline void setB(uint8_t value) { BC = (BC & 0x00FF) | (value << 8); }
    inline void setC(uint8_t value) { BC = (BC & 0xFF00) | value; }
    inline void setD(uint8_t value) { DE = (DE & 0x00FF) | (value << 8); }
    inline void setE(uint8_t value) { DE = (DE & 0xFF00) | value; }
    inline void setH(uint8_t value) { HL = (HL & 0x00FF) | (value << 8); }
    inline void setL(uint8_t value) { HL = (HL & 0xFF00) | value; }

    // Flags helper functions
    inline uint8_t getFlagZ() const { return AF & 0x80; }
    inline uint8_t getFlagN() const { return AF & 0x40; }
    inline uint8_t getFlagH() const { return AF & 0x20; }
    inline uint8_t getFlagC() const { return AF & 0x10; }
    inline void setFlagZ(uint8_t value) { AF = (AF & 0xFF7F) | (value << 7); }
    inline void setFlagN(uint8_t value) { AF = (AF & 0xFFBF) | (value << 6); }
    inline void setFlagH(uint8_t value) { AF = (AF & 0xFFDF) | (value << 5); }
    inline void setFlagC(uint8_t value) { AF = (AF & 0xFFEF) | (value << 4); }

    std::unordered_map<InstructionDecoder::Op, uint8_t (GameBoyEmulator::*)(), InstructionDecoder::OpHash> op_handlers;
    std::unordered_map<InstructionDecoder::Op, uint8_t (GameBoyEmulator::*)(), InstructionDecoder::OpHash> cb_handlers;

    static GameBoyEmulator* instance;
    std::string filepath;

    GameBoyEmulator();
    GameBoyEmulator(GameBoyEmulator&) = delete;
    GameBoyEmulator(GameBoyEmulator&&) = delete;

    uint8_t fetchOpcode();
    
    // CB prefix instruction handler
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
    
    // 8-bit arithmetic and logical instructions
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
    
    // 16-bit arithmetic instructions
    uint8_t op_inc_rr();
    uint8_t op_dec_rr();
    uint8_t op_add_hl_rr();
    uint8_t op_add_sp_e();
    
    // Rotate, shift, and bit operation instructions
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
    
    // Control flow instructions
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
    
    // Miscellaneous instructions
    uint8_t op_halt();
    uint8_t op_stop();
    uint8_t op_di();
    uint8_t op_ei();
    uint8_t op_nop();

public:

    void emulate();
    void setFilepath(const std::string& filepath);
    static GameBoyEmulator* getInstance();

};

#endif