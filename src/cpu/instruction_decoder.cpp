#include "instruction_decoder.hpp"
#include "cpu.hpp"

void InstructionDecoder::initializeHandlers(CPU* cpu) {
    registerInstructions(cpu);
    registerCbInstructions(cpu);
}

void InstructionDecoder::registerInstructions(CPU* cpu) {
    auto& h = cpu->op_handlers_;

    // Broadest patterns first (fewer fixed bits), narrower ones overwrite.
    // mask 0xC0 = 2 fixed bits -> 64 opcodes each
    add_instruction(h, 0xC0, 0x40, &CPU::op_ld_r_r);

    // mask 0xC7 = 5 fixed bits -> 32 opcodes each
    add_instruction(h, 0xC7, 0x04, &CPU::op_inc_r);
    add_instruction(h, 0xC7, 0x05, &CPU::op_dec_r);
    add_instruction(h, 0xC7, 0x06, &CPU::op_ld_r_imm);
    add_instruction(h, 0xC7, 0xC7, &CPU::op_rst_imm);

    // mask 0xCF = 6 fixed bits -> 4 opcodes each
    add_instruction(h, 0xCF, 0x01, &CPU::op_ld_rr_imm);
    add_instruction(h, 0xCF, 0x03, &CPU::op_inc_rr);
    add_instruction(h, 0xCF, 0x09, &CPU::op_add_hl_rr);
    add_instruction(h, 0xCF, 0x0B, &CPU::op_dec_rr);
    add_instruction(h, 0xCF, 0xC1, &CPU::op_pop_rr);
    add_instruction(h, 0xCF, 0xC5, &CPU::op_push_rr);

    // mask 0xE7 = 6 fixed bits -> 4 opcodes each
    add_instruction(h, 0xE7, 0x20, &CPU::op_jr_cc_e);
    add_instruction(h, 0xE7, 0xC0, &CPU::op_ret_cc);
    add_instruction(h, 0xE7, 0xC2, &CPU::op_jp_cc_imm);
    add_instruction(h, 0xE7, 0xC4, &CPU::op_call_cc_imm);

    // mask 0xF8 = 5 fixed bits -> 8 opcodes each
    add_instruction(h, 0xF8, 0x70, &CPU::op_ld_hl_ind_r);
    add_instruction(h, 0xF8, 0x80, &CPU::op_add_r);
    add_instruction(h, 0xF8, 0x88, &CPU::op_adc_r);
    add_instruction(h, 0xF8, 0x90, &CPU::op_sub_r);
    add_instruction(h, 0xF8, 0x98, &CPU::op_sbc_r);
    add_instruction(h, 0xF8, 0xA0, &CPU::op_and_r);
    add_instruction(h, 0xF8, 0xA8, &CPU::op_xor_r);
    add_instruction(h, 0xF8, 0xB0, &CPU::op_or_r);
    add_instruction(h, 0xF8, 0xB8, &CPU::op_cp_r);

    // Exact matches (mask 0xFF) - override any broader pattern
    add_instruction(h, 0xFF, 0x00, &CPU::op_nop);
    add_instruction(h, 0xFF, 0x02, &CPU::op_ld_bc_ind_a);
    add_instruction(h, 0xFF, 0x07, &CPU::op_rlca);
    add_instruction(h, 0xFF, 0x08, &CPU::op_ld_imm_ind_sp);
    add_instruction(h, 0xFF, 0x0A, &CPU::op_ld_a_bc_ind);
    add_instruction(h, 0xFF, 0x0F, &CPU::op_rrca);
    add_instruction(h, 0xFF, 0x10, &CPU::op_stop);
    add_instruction(h, 0xFF, 0x12, &CPU::op_ld_de_ind_a);
    add_instruction(h, 0xFF, 0x17, &CPU::op_rla);
    add_instruction(h, 0xFF, 0x18, &CPU::op_jr_e);
    add_instruction(h, 0xFF, 0x1A, &CPU::op_ld_a_de_ind);
    add_instruction(h, 0xFF, 0x1F, &CPU::op_rra);
    add_instruction(h, 0xFF, 0x22, &CPU::op_ld_hl_ind_inc_a);
    add_instruction(h, 0xFF, 0x27, &CPU::op_daa);
    add_instruction(h, 0xFF, 0x2A, &CPU::op_ld_a_hl_ind_inc);
    add_instruction(h, 0xFF, 0x2F, &CPU::op_cpl);
    add_instruction(h, 0xFF, 0x32, &CPU::op_ld_hl_ind_dec_a);
    add_instruction(h, 0xFF, 0x36, &CPU::op_ld_hl_ind_imm);
    add_instruction(h, 0xFF, 0x37, &CPU::op_scf);
    add_instruction(h, 0xFF, 0x3A, &CPU::op_ld_a_hl_ind_dec);
    add_instruction(h, 0xFF, 0x3F, &CPU::op_ccf);
    add_instruction(h, 0xFF, 0x76, &CPU::op_halt);
    add_instruction(h, 0xFF, 0xC3, &CPU::op_jp_imm);
    add_instruction(h, 0xFF, 0xC6, &CPU::op_add_imm);
    add_instruction(h, 0xFF, 0xC9, &CPU::op_ret);
    add_instruction(h, 0xFF, 0xCB, &CPU::cb_ins_handler);
    add_instruction(h, 0xFF, 0xCD, &CPU::op_call_imm);
    add_instruction(h, 0xFF, 0xCE, &CPU::op_adc_imm);
    add_instruction(h, 0xFF, 0xD6, &CPU::op_sub_imm);
    add_instruction(h, 0xFF, 0xD9, &CPU::op_reti);
    add_instruction(h, 0xFF, 0xDE, &CPU::op_sbc_imm);
    add_instruction(h, 0xFF, 0xE0, &CPU::op_ldh_imm_ind_a);
    add_instruction(h, 0xFF, 0xE2, &CPU::op_ldh_c_ind_a);
    add_instruction(h, 0xFF, 0xE6, &CPU::op_and_imm);
    add_instruction(h, 0xFF, 0xE8, &CPU::op_add_sp_e);
    add_instruction(h, 0xFF, 0xE9, &CPU::op_jp_hl);
    add_instruction(h, 0xFF, 0xEA, &CPU::op_ld_imm_ind_a);
    add_instruction(h, 0xFF, 0xEE, &CPU::op_xor_imm);
    add_instruction(h, 0xFF, 0xF0, &CPU::op_ldh_a_imm_ind);
    add_instruction(h, 0xFF, 0xF2, &CPU::op_ldh_a_c_ind);
    add_instruction(h, 0xFF, 0xF3, &CPU::op_di);
    add_instruction(h, 0xFF, 0xF6, &CPU::op_or_imm);
    add_instruction(h, 0xFF, 0xF8, &CPU::op_ld_hl_sp_e);
    add_instruction(h, 0xFF, 0xF9, &CPU::op_ld_sp_hl);
    add_instruction(h, 0xFF, 0xFA, &CPU::op_ld_a_imm_ind);
    add_instruction(h, 0xFF, 0xFB, &CPU::op_ei);
    add_instruction(h, 0xFF, 0xFE, &CPU::op_cp_imm);
}

void InstructionDecoder::registerCbInstructions(CPU* cpu) {
    auto& h = cpu->cb_handlers_;

    // Broadest first (mask 0xC0 -> 64 opcodes each)
    add_instruction(h, 0xC0, 0x40, &CPU::op_bit_b_r);
    add_instruction(h, 0xC0, 0x80, &CPU::op_res_b_r);
    add_instruction(h, 0xC0, 0xC0, &CPU::op_set_b_r);

    // mask 0xC7 -> 8 opcodes each (the (HL) variants of BIT/RES/SET)
    add_instruction(h, 0xC7, 0x46, &CPU::op_bit_b_hl_ind);
    add_instruction(h, 0xC7, 0x86, &CPU::op_res_b_hl_ind);
    add_instruction(h, 0xC7, 0xC6, &CPU::op_set_b_hl_ind);

    // mask 0xF8 -> 8 opcodes each (rotate/shift register variants)
    add_instruction(h, 0xF8, 0x00, &CPU::op_rlc_r);
    add_instruction(h, 0xF8, 0x08, &CPU::op_rrc_r);
    add_instruction(h, 0xF8, 0x10, &CPU::op_rl_r);
    add_instruction(h, 0xF8, 0x18, &CPU::op_rr_r);
    add_instruction(h, 0xF8, 0x20, &CPU::op_sla_r);
    add_instruction(h, 0xF8, 0x28, &CPU::op_sra_r);
    add_instruction(h, 0xF8, 0x30, &CPU::op_swap_r);
    add_instruction(h, 0xF8, 0x38, &CPU::op_srl_r);

    // Exact matches (mask 0xFF) - the (HL) variants of rotate/shift
    add_instruction(h, 0xFF, 0x06, &CPU::op_rlc_hl_ind);
    add_instruction(h, 0xFF, 0x0E, &CPU::op_rrc_hl_ind);
    add_instruction(h, 0xFF, 0x16, &CPU::op_rl_hl_ind);
    add_instruction(h, 0xFF, 0x1E, &CPU::op_rr_hl_ind);
    add_instruction(h, 0xFF, 0x26, &CPU::op_sla_hl_ind);
    add_instruction(h, 0xFF, 0x2E, &CPU::op_sra_hl_ind);
    add_instruction(h, 0xFF, 0x36, &CPU::op_swap_hl_ind);
    add_instruction(h, 0xFF, 0x3E, &CPU::op_srl_hl_ind);
}

void InstructionDecoder::add_instruction(std::array<Handler, 256>& handlers, u8 mask, u8 pattern, Handler instruction) {
    int index = find_first_zero(mask);

    if (index == 8) {
        handlers[pattern] = instruction;
        return;
    }

    add_instruction(handlers, mask | (1 << index), pattern, instruction);
    add_instruction(handlers, mask | (1 << index), pattern | (1 << index), instruction);
}

u8 InstructionDecoder::find_first_zero(u8 value) {
    for (int i = 0; i < 8; i++) {
        if (!test_bit(value, i)) {
            return i;
        }
    }
    return 8;
}
