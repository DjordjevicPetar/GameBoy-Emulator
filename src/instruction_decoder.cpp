#include "../inc/instruction_decoder.h"
#include "../inc/game_boy_emulator.h"

void InstructionDecoder::initializeHandlers(GameBoyEmulator* emulator) {
    registerInstructions(emulator);
    registerCbInstructions(emulator);
}

void InstructionDecoder::registerInstructions(GameBoyEmulator* emulator) {
    // Access op_handlers through friend class access
    auto& handlers = emulator->op_handlers;
    
    // Register all instruction handlers here
    // Op(mask, pattern) - use 0xFF mask to match exact opcode value
    // Direct member function pointers to GameBoyEmulator methods
    handlers = {
        // 8-bit load instructions
        {Op(0xC0, 0x40), &GameBoyEmulator::op_ld_r_r},
        {Op(0xC7, 0x06), &GameBoyEmulator::op_ld_r_imm},
        {Op(0xC7, 0x46), &GameBoyEmulator::op_ld_r_hl_ind},
        {Op(0xF8, 0x70), &GameBoyEmulator::op_ld_hl_ind_r},
        {Op(0xFF, 0x36), &GameBoyEmulator::op_ld_hl_ind_imm},
        {Op(0xFF, 0x0A), &GameBoyEmulator::op_ld_a_bc_ind},
        {Op(0xFF, 0x1A), &GameBoyEmulator::op_ld_a_de_ind},
        {Op(0xFF, 0x02), &GameBoyEmulator::op_ld_bc_ind_a},
        {Op(0xFF, 0x12), &GameBoyEmulator::op_ld_de_ind_a},
        {Op(0xFF, 0xFA), &GameBoyEmulator::op_ld_a_imm_ind},
        {Op(0xFF, 0xEA), &GameBoyEmulator::op_ld_imm_ind_a},
        {Op(0xFF, 0xF2), &GameBoyEmulator::op_ldh_a_c_ind},
        {Op(0xFF, 0xE2), &GameBoyEmulator::op_ldh_c_ind_a},
        {Op(0xFF, 0xF0), &GameBoyEmulator::op_ldh_a_imm_ind},
        {Op(0xFF, 0xE0), &GameBoyEmulator::op_ldh_imm_ind_a},
        {Op(0xFF, 0x3A), &GameBoyEmulator::op_ld_a_hl_ind_dec},
        {Op(0xFF, 0x32), &GameBoyEmulator::op_ld_hl_ind_dec_a},
        {Op(0xFF, 0x2A), &GameBoyEmulator::op_ld_a_hl_ind_inc},
        {Op(0xFF, 0x22), &GameBoyEmulator::op_ld_hl_ind_inc_a},
        // 16-bit load instructions
        {Op(0xCF, 0x01), &GameBoyEmulator::op_ld_rr_imm},
        {Op(0xFF, 0x08), &GameBoyEmulator::op_ld_imm_ind_sp},
        {Op(0xFF, 0xF9), &GameBoyEmulator::op_ld_sp_hl},
        {Op(0xCF, 0xC5), &GameBoyEmulator::op_push_rr},
        {Op(0xCF, 0xC1), &GameBoyEmulator::op_pop_rr},
        {Op(0xFF, 0xF8), &GameBoyEmulator::op_ld_hl_sp_e},
        // 8-bit arithmetic and logical instructions
        {Op(0xF8, 0x80), &GameBoyEmulator::op_add_r},
        {Op(0xFF, 0x86), &GameBoyEmulator::op_adc_hl_ind},
        {Op(0xFF, 0xC6), &GameBoyEmulator::op_add_imm},
        {Op(0xF8, 0x88), &GameBoyEmulator::op_adc_r},
        {Op(0xFF, 0x8E), &GameBoyEmulator::op_adc_hl_ind},
        {Op(0xFF, 0xCE), &GameBoyEmulator::op_adc_imm},
        {Op(0xF8, 0x90), &GameBoyEmulator::op_sub_r},
        {Op(0xFF, 0x96), &GameBoyEmulator::op_sub_hl_ind},
        {Op(0xFF, 0xD6), &GameBoyEmulator::op_sub_imm},
        {Op(0xF8, 0x98), &GameBoyEmulator::op_sbc_r},
        {Op(0xFF, 0x9E), &GameBoyEmulator::op_sbc_hl_ind},
        {Op(0xFF, 0xDE), &GameBoyEmulator::op_sbc_imm},
        {Op(0xF8, 0xB8), &GameBoyEmulator::op_cp_r},
        {Op(0xFF, 0xBE), &GameBoyEmulator::op_sbc_hl_ind},
        {Op(0xFF, 0xFE), &GameBoyEmulator::op_sbc_imm},
        {Op(0xC7, 0x04), &GameBoyEmulator::op_inc_r},
        {Op(0xFF, 0x34), &GameBoyEmulator::op_inc_hl_ind},
        {Op(0xC7, 0x05), &GameBoyEmulator::op_dec_r},
        {Op(0xFF, 0x35), &GameBoyEmulator::op_dec_hl_ind},
        {Op(0xF8, 0xA0), &GameBoyEmulator::op_and_r},
        {Op(0xFF, 0xA6), &GameBoyEmulator::op_and_hl_ind},
        {Op(0xFF, 0xE6), &GameBoyEmulator::op_and_imm},
        {Op(0xF8, 0xB0), &GameBoyEmulator::op_or_r},
        {Op(0xFF, 0xB6), &GameBoyEmulator::op_or_hl_ind},
        {Op(0xFF, 0xF6), &GameBoyEmulator::op_or_imm},
        {Op(0xF8, 0xA8), &GameBoyEmulator::op_xor_r},
        {Op(0xFF, 0xAE), &GameBoyEmulator::op_xor_hl_ind},
        {Op(0xFF, 0xEE), &GameBoyEmulator::op_xor_imm},
        {Op(0xFF, 0x3F), &GameBoyEmulator::op_ccf},
        {Op(0xFF, 0x37), &GameBoyEmulator::op_scf},
        {Op(0xFF, 0x27), &GameBoyEmulator::op_daa},
        {Op(0xFF, 0x2F), &GameBoyEmulator::op_cpl},
        // 16-bit arithmetic instructions
        {Op(0xCF, 0x03), &GameBoyEmulator::op_inc_rr},
        {Op(0xCF, 0x0B), &GameBoyEmulator::op_dec_rr},
        {Op(0xCF, 0x09), &GameBoyEmulator::op_add_hl_rr},
        {Op(0xFF, 0xE8), &GameBoyEmulator::op_add_sp_e},
        // Rotate, shift, and bit operation instructions
        {Op(0xFF, 0x07), &GameBoyEmulator::op_rlca},
        {Op(0xFF, 0x0F), &GameBoyEmulator::op_rrca},
        {Op(0xFF, 0x17), &GameBoyEmulator::op_rla},
        {Op(0xFF, 0x1F), &GameBoyEmulator::op_rra},
        {Op(0xFF, 0xCB), &GameBoyEmulator::cb_ins_handler}, // A lot of instructions are being called with this prefix
        // Control flow instructions
        {Op(0xFF, 0xC3), &GameBoyEmulator::op_jp_imm},
        {Op(0xFF, 0xE9), &GameBoyEmulator::op_jp_hl},
        {Op(0xE7, 0xC2), &GameBoyEmulator::op_jp_cc_imm},
        {Op(0xFF, 0x18), &GameBoyEmulator::op_jr_e},
        {Op(0xE7, 0x20), &GameBoyEmulator::op_jr_cc_e},
        {Op(0xFF, 0xCD), &GameBoyEmulator::op_call_imm},
        {Op(0xE7, 0xC4), &GameBoyEmulator::op_call_cc_imm},
        {Op(0xFF, 0xC9), &GameBoyEmulator::op_ret},
        {Op(0xE7, 0xC0), &GameBoyEmulator::op_ret_cc},
        {Op(0xFF, 0xD9), &GameBoyEmulator::op_reti},
        {Op(0xC7, 0xC7), &GameBoyEmulator::op_rst_imm},
        // Miscellanous instructions
        {Op(0xFF, 0x76), &GameBoyEmulator::op_halt},
        {Op(0xFF, 0x10), &GameBoyEmulator::op_stop},
        {Op(0xFF, 0xF3), &GameBoyEmulator::op_di},
        {Op(0xFF, 0xFB), &GameBoyEmulator::op_ei},
        {Op(0xFF, 0x00), &GameBoyEmulator::op_nop},
    };
}

void InstructionDecoder::registerCbInstructions(GameBoyEmulator* emulator) {
    // Access cb_handlers through friend class access
    auto& handlers = emulator->cb_handlers;
    
    // Register all CB prefix instruction handlers here
    // Op(mask, pattern) - use 0xFF mask to match exact opcode value
    // Direct member function pointers to GameBoyEmulator methods
    handlers = {
        {Op(0xF8, 0x00), &GameBoyEmulator::op_rlc_r},
        {Op(0xFF, 0x06), &GameBoyEmulator::op_rlc_hl_ind},
        {Op(0xF8, 0x08), &GameBoyEmulator::op_rrc_r},
        {Op(0xFF, 0x0E), &GameBoyEmulator::op_rrc_hl_ind},
        {Op(0xF8, 0x10), &GameBoyEmulator::op_rl_r},
        {Op(0xFF, 0x16), &GameBoyEmulator::op_rl_hl_ind},
        {Op(0xF8, 0x18), &GameBoyEmulator::op_rr_r},
        {Op(0xFF, 0x1E), &GameBoyEmulator::op_rr_hl_ind},
        {Op(0xF8, 0x20), &GameBoyEmulator::op_sla_r},
        {Op(0xFF, 0x26), &GameBoyEmulator::op_sla_hl_ind},
        {Op(0xF8, 0x28), &GameBoyEmulator::op_sra_r},
        {Op(0xFF, 0x2E), &GameBoyEmulator::op_sra_hl_ind},
        {Op(0xF8, 0x30), &GameBoyEmulator::op_swap_r},
        {Op(0xFF, 0x36), &GameBoyEmulator::op_swap_hl_ind},
        {Op(0xF8, 0x38), &GameBoyEmulator::op_srl_r},
        {Op(0xFF, 0x3E), &GameBoyEmulator::op_srl_hl_ind},
        {Op(0xC0, 0x40), &GameBoyEmulator::op_bit_b_r},
        {Op(0xC7, 0x46), &GameBoyEmulator::op_bit_b_hl_ind},
        {Op(0xC0, 0x80), &GameBoyEmulator::op_res_b_r},
        {Op(0xC7, 0x86), &GameBoyEmulator::op_res_b_hl_ind},
        {Op(0xC0, 0xC0), &GameBoyEmulator::op_set_b_r},
        {Op(0xC7, 0xC6), &GameBoyEmulator::op_set_b_hl_ind},
    };
}
