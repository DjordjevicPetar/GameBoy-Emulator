#ifndef EMULATOR_H_
#define EMULATOR_H_

#include "../inc/constants.h"
#include "../inc/instruction_decoder.h"
#include <unordered_map>
#include <string>

// Forward declaration
class InstructionDecoder;

class GameBoyEmulator {
    // Allow InstructionDecoder to access private members
    friend class InstructionDecoder;

private:

    uint8_t current_opcode;

    // Memory
    uint8_t memory[MEMORY_SIZE];
    uint8_t read_memory(uint16_t address) const;
    void write_memory(uint16_t address, uint8_t value);
    uint16_t endian_swap(uint8_t value1, uint8_t value2) const;

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

    std::unordered_map<InstructionDecoder::Op, void (GameBoyEmulator::*)(), InstructionDecoder::OpHash> op_handlers;
    std::unordered_map<InstructionDecoder::Op, void (GameBoyEmulator::*)(), InstructionDecoder::OpHash> cb_handlers;

    static GameBoyEmulator* instance;
    std::string filepath;

    GameBoyEmulator();
    GameBoyEmulator(GameBoyEmulator&) = delete;
    GameBoyEmulator(GameBoyEmulator&&) = delete;

    uint8_t fetchOpcode();
    
    // CB prefix instruction handler
    void cb_ins_handler();
    
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
    
    // 8-bit arithmetic and logical instructions
    void op_add_r();
    void op_adc_hl_ind();
    void op_add_imm();
    void op_adc_r();
    void op_adc_imm();
    void op_sub_r();
    void op_sub_hl_ind();
    void op_sub_imm();
    void op_sbc_r();
    void op_sbc_hl_ind();
    void op_sbc_imm();
    void op_cp_r();
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
    
    // 16-bit arithmetic instructions
    void op_inc_rr();
    void op_dec_rr();
    void op_add_hl_rr();
    void op_add_sp_e();
    
    // Rotate, shift, and bit operation instructions
    void op_rlca();
    void op_rrca();
    void op_rla();
    void op_rra();
    
    // Control flow instructions
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
    
    // Miscellaneous instructions
    void op_halt();
    void op_stop();
    void op_di();
    void op_ei();
    void op_nop();
    
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

public:

    void emulate();
    void setFilepath(const std::string& filepath);
    GameBoyEmulator* getInstance();

};

#endif