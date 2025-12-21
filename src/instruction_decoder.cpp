#include "../inc/instruction_decoder.hpp"
#include "../inc/cpu.hpp"

void InstructionDecoder::initializeHandlers(CPU* cpu) {
    registerInstructions(cpu);
    registerCbInstructions(cpu);
}

void InstructionDecoder::registerInstructions(CPU* cpu) {
    // Access op_handlers through friend class access
    auto& handlers = cpu->op_handlers;
    
    // Register all instruction handlers here
    // Op(mask, pattern) - use 0xFF mask to match exact opcode value
    // Direct member function pointers to CPU methods
    handlers = {
        // 8-bit load instructions
        {Op(0xC0, 0x40), &CPU::op_ld_r_r},
        {Op(0xC7, 0x06), &CPU::op_ld_r_imm},
        {Op(0xC7, 0x46), &CPU::op_ld_r_hl_ind},
        {Op(0xF8, 0x70), &CPU::op_ld_hl_ind_r},
        {Op(0xFF, 0x36), &CPU::op_ld_hl_ind_imm},
        {Op(0xFF, 0x0A), &CPU::op_ld_a_bc_ind},
        {Op(0xFF, 0x1A), &CPU::op_ld_a_de_ind},
        {Op(0xFF, 0x02), &CPU::op_ld_bc_ind_a},
        {Op(0xFF, 0x12), &CPU::op_ld_de_ind_a},
        {Op(0xFF, 0xFA), &CPU::op_ld_a_imm_ind},
        {Op(0xFF, 0xEA), &CPU::op_ld_imm_ind_a},
        {Op(0xFF, 0xF2), &CPU::op_ldh_a_c_ind},
        {Op(0xFF, 0xE2), &CPU::op_ldh_c_ind_a},
        {Op(0xFF, 0xF0), &CPU::op_ldh_a_imm_ind},
        {Op(0xFF, 0xE0), &CPU::op_ldh_imm_ind_a},
        {Op(0xFF, 0x3A), &CPU::op_ld_a_hl_ind_dec},
        {Op(0xFF, 0x32), &CPU::op_ld_hl_ind_dec_a},
        {Op(0xFF, 0x2A), &CPU::op_ld_a_hl_ind_inc},
        {Op(0xFF, 0x22), &CPU::op_ld_hl_ind_inc_a},
        // 16-bit load instructions
        {Op(0xCF, 0x01), &CPU::op_ld_rr_imm},
        {Op(0xFF, 0x08), &CPU::op_ld_imm_ind_sp},
        {Op(0xFF, 0xF9), &CPU::op_ld_sp_hl},
        {Op(0xCF, 0xC5), &CPU::op_push_rr},
        {Op(0xCF, 0xC1), &CPU::op_pop_rr},
        {Op(0xFF, 0xF8), &CPU::op_ld_hl_sp_e},
        // 8-bit arithmetic and logical instructions
        {Op(0xF8, 0x80), &CPU::op_add_r},
        {Op(0xFF, 0x86), &CPU::op_adc_hl_ind},
        {Op(0xFF, 0xC6), &CPU::op_add_imm},
        {Op(0xF8, 0x88), &CPU::op_adc_r},
        {Op(0xFF, 0x8E), &CPU::op_adc_hl_ind},
        {Op(0xFF, 0xCE), &CPU::op_adc_imm},
        {Op(0xF8, 0x90), &CPU::op_sub_r},
        {Op(0xFF, 0x96), &CPU::op_sub_hl_ind},
        {Op(0xFF, 0xD6), &CPU::op_sub_imm},
        {Op(0xF8, 0x98), &CPU::op_sbc_r},
        {Op(0xFF, 0x9E), &CPU::op_sbc_hl_ind},
        {Op(0xFF, 0xDE), &CPU::op_sbc_imm},
        {Op(0xF8, 0xB8), &CPU::op_cp_r},
        {Op(0xFF, 0xBE), &CPU::op_sbc_hl_ind},
        {Op(0xFF, 0xFE), &CPU::op_sbc_imm},
        {Op(0xC7, 0x04), &CPU::op_inc_r},
        {Op(0xFF, 0x34), &CPU::op_inc_hl_ind},
        {Op(0xC7, 0x05), &CPU::op_dec_r},
        {Op(0xFF, 0x35), &CPU::op_dec_hl_ind},
        {Op(0xF8, 0xA0), &CPU::op_and_r},
        {Op(0xFF, 0xA6), &CPU::op_and_hl_ind},
        {Op(0xFF, 0xE6), &CPU::op_and_imm},
        {Op(0xF8, 0xB0), &CPU::op_or_r},
        {Op(0xFF, 0xB6), &CPU::op_or_hl_ind},
        {Op(0xFF, 0xF6), &CPU::op_or_imm},
        {Op(0xF8, 0xA8), &CPU::op_xor_r},
        {Op(0xFF, 0xAE), &CPU::op_xor_hl_ind},
        {Op(0xFF, 0xEE), &CPU::op_xor_imm},
        {Op(0xFF, 0x3F), &CPU::op_ccf},
        {Op(0xFF, 0x37), &CPU::op_scf},
        {Op(0xFF, 0x27), &CPU::op_daa},
        {Op(0xFF, 0x2F), &CPU::op_cpl},
        // 16-bit arithmetic instructions
        {Op(0xCF, 0x03), &CPU::op_inc_rr},
        {Op(0xCF, 0x0B), &CPU::op_dec_rr},
        {Op(0xCF, 0x09), &CPU::op_add_hl_rr},
        {Op(0xFF, 0xE8), &CPU::op_add_sp_e},
        // Rotate, shift, and bit operation instructions
        {Op(0xFF, 0x07), &CPU::op_rlca},
        {Op(0xFF, 0x0F), &CPU::op_rrca},
        {Op(0xFF, 0x17), &CPU::op_rla},
        {Op(0xFF, 0x1F), &CPU::op_rra},
        {Op(0xFF, 0xCB), &CPU::cb_ins_handler}, // A lot of instructions are being called with this prefix
        // Control flow instructions
        {Op(0xFF, 0xC3), &CPU::op_jp_imm},
        {Op(0xFF, 0xE9), &CPU::op_jp_hl},
        {Op(0xE7, 0xC2), &CPU::op_jp_cc_imm},
        {Op(0xFF, 0x18), &CPU::op_jr_e},
        {Op(0xE7, 0x20), &CPU::op_jr_cc_e},
        {Op(0xFF, 0xCD), &CPU::op_call_imm},
        {Op(0xE7, 0xC4), &CPU::op_call_cc_imm},
        {Op(0xFF, 0xC9), &CPU::op_ret},
        {Op(0xE7, 0xC0), &CPU::op_ret_cc},
        {Op(0xFF, 0xD9), &CPU::op_reti},
        {Op(0xC7, 0xC7), &CPU::op_rst_imm},
        // Miscellanous instructions
        {Op(0xFF, 0x76), &CPU::op_halt},
        {Op(0xFF, 0x10), &CPU::op_stop},
        {Op(0xFF, 0xF3), &CPU::op_di},
        {Op(0xFF, 0xFB), &CPU::op_ei},
        {Op(0xFF, 0x00), &CPU::op_nop}
    };
}

void InstructionDecoder::registerCbInstructions(CPU* cpu) {
    // Access cb_handlers through friend class access
    auto& handlers = cpu->cb_handlers;
    
    // Register all CB prefix instruction handlers here
    // Op(mask, pattern) - use 0xFF mask to match exact opcode value
    // Direct member function pointers to CPU methods
    handlers = {
        {Op(0xF8, 0x00), &CPU::op_rlc_r},
        {Op(0xFF, 0x06), &CPU::op_rlc_hl_ind},
        {Op(0xF8, 0x08), &CPU::op_rrc_r},
        {Op(0xFF, 0x0E), &CPU::op_rrc_hl_ind},
        {Op(0xF8, 0x10), &CPU::op_rl_r},
        {Op(0xFF, 0x16), &CPU::op_rl_hl_ind},
        {Op(0xF8, 0x18), &CPU::op_rr_r},
        {Op(0xFF, 0x1E), &CPU::op_rr_hl_ind},
        {Op(0xF8, 0x20), &CPU::op_sla_r},
        {Op(0xFF, 0x26), &CPU::op_sla_hl_ind},
        {Op(0xF8, 0x28), &CPU::op_sra_r},
        {Op(0xFF, 0x2E), &CPU::op_sra_hl_ind},
        {Op(0xF8, 0x30), &CPU::op_swap_r},
        {Op(0xFF, 0x36), &CPU::op_swap_hl_ind},
        {Op(0xF8, 0x38), &CPU::op_srl_r},
        {Op(0xFF, 0x3E), &CPU::op_srl_hl_ind},
        {Op(0xC0, 0x40), &CPU::op_bit_b_r},
        {Op(0xC7, 0x46), &CPU::op_bit_b_hl_ind},
        {Op(0xC0, 0x80), &CPU::op_res_b_r},
        {Op(0xC7, 0x86), &CPU::op_res_b_hl_ind},
        {Op(0xC0, 0xC0), &CPU::op_set_b_r},
        {Op(0xC7, 0xC6), &CPU::op_set_b_hl_ind},
    };
}
