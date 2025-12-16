#include "../inc/game_boy_emulator.h"
#include "../inc/instruction_decoder.h"

GameBoyEmulator* GameBoyEmulator::instance = nullptr;

GameBoyEmulator::GameBoyEmulator() {
    // Initialize instruction handlers through InstructionDecoder
    InstructionDecoder::initializeHandlers(this);
}

GameBoyEmulator* GameBoyEmulator::getInstance() {

    if (instance == nullptr) {
        instance = new GameBoyEmulator();
    }

    return instance;
}

void GameBoyEmulator::setFilepath(const std::string& filepath) {
    this->filepath = filepath;
}

void GameBoyEmulator::emulate() {
    
    while (true) {
        current_opcode = fetchOpcode();
        bool is_recognized = false;
        
        for (auto& entry : op_handlers) {
            if ((current_opcode & entry.first.mask) == entry.first.pattern) {
                (this->*(entry.second))();
                is_recognized = true;
                break;
            }
        }
        
        // TODO: Handle undefined opcodes
        if (!is_recognized) {
            // Handle undefined opcode
        }
    }
}

uint8_t GameBoyEmulator::fetchOpcode() {
    uint8_t opcode = memory[PC];
    PC++;
    return opcode;
}

void GameBoyEmulator::cb_ins_handler() {
    // Read the next byte after 0xCB (the actual CB instruction opcode)
    current_opcode = fetchOpcode();
    bool is_recognized = false;
    
    for (auto& entry : cb_handlers) {
        if ((current_opcode & entry.first.mask) == entry.first.pattern) {
            (this->*(entry.second))();
            is_recognized = true;
            break;
        }
    }
    
    // TODO: Handle undefined CB opcodes
    if (!is_recognized) {
        // Handle undefined CB opcode
    }
}

uint16_t GameBoyEmulator::endian_swap(uint8_t value1, uint8_t value2) const {
    return (static_cast<uint16_t>(value2) << 8) | static_cast<uint16_t>(value1);
}

inline uint8_t GameBoyEmulator::read_first_register_8_bit_parameter() const {
    // Source register is in bits 5-3
    return (current_opcode >> 3) & 0x07;
}

inline uint8_t GameBoyEmulator::read_second_register_8_bit_parameter() const {
    // Destination register is in bits 2-0
    return current_opcode & 0x07;
}

inline uint8_t GameBoyEmulator::read_first_register_16_bit_parameter() const {
    // Source register is in bits 5-4
    return (current_opcode >> 4) & 0x03;
}

inline uint8_t GameBoyEmulator::read_second_register_16_bit_parameter() const {
    // Destination register is in bits 3-2
    return (current_opcode >> 2) & 0x03;
}

inline uint8_t GameBoyEmulator::read_register_8_bit(uint8_t register_number) const {
    switch (register_number) {
        case 0: return getB();
        case 1: return getC();
        case 2: return getD();
        case 3: return getE();
        case 4: return getH();
        case 5: return getL();
        case 6: return read_memory(HL);  // (HL) - indirect
        case 7: return getA();
        default: throw std::runtime_error("Invalid register number");
    }
}

inline void GameBoyEmulator::write_register_8_bit(uint8_t register_number, uint8_t value) {
    switch (register_number) {
        case 0: setB(value); break;
        case 1: setC(value); break;
        case 2: setD(value); break;
        case 3: setE(value); break;
        case 4: setH(value); break;
        case 5: setL(value); break;
        case 6: write_memory(HL, value); break;  // (HL) - indirect
        case 7: setA(value); break;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline uint16_t GameBoyEmulator::read_register_16_bit(uint8_t register_number) const {
    switch (register_number) {
        case 0: return BC;
        case 1: return DE;
        case 2: return HL;
        case 3: return SP;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline void GameBoyEmulator::write_register_16_bit(uint8_t register_number, uint16_t value) {
    switch (register_number) {
        case 0: BC = value; break;
        case 1: DE = value; break;
        case 2: HL = value; break;
        case 3: SP = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline uint16_t GameBoyEmulator::read_register_16_bit_stack(uint8_t register_number) const {
    switch (register_number) {
        case 0: return BC;
        case 1: return DE;
        case 2: return HL;
        case 3: return AF;
        default: throw std::runtime_error("Invalid register number");
    }
}

inline void GameBoyEmulator::write_register_16_bit_stack(uint8_t register_number, uint16_t value) {
    switch (register_number) {
        case 0: BC = value; break;
        case 1: DE = value; break;
        case 2: HL = value; break;
        case 3: AF = value; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

uint16_t GameBoyEmulator::read_register_16_bit_memory(uint8_t register_number) {
    switch (register_number) {
        case 0: return BC;
        case 1: return DE;
        case 2: {
            uint16_t value = HL;
            HL++;
            return value;
        }
        case 3: {
            uint16_t value = HL;
            HL--;
            return value;
        }
        default: throw std::runtime_error("Invalid register number");
    }
}

void GameBoyEmulator::write_register_16_bit_memory(uint8_t register_number, uint16_t value) {
    switch (register_number) {
        case 0: BC = value; break;
        case 1: DE = value; break;
        case 2: HL = value; HL++; break;
        case 3: HL = value; HL--; break;
        default: throw std::runtime_error("Invalid register number");
    }
}

// 8-bit load instructions
void GameBoyEmulator::op_ld_r_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint8_t value = read_register_8_bit(source_register);
    write_register_8_bit(destination_register, value);
    return 4; // 4 cycles
}

void GameBoyEmulator::op_ld_r_imm() {
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint8_t value = fetchOpcode();
    write_register_8_bit(destination_register, value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_r_hl_ind() {
    uint8_t destination_register = read_first_register_8_bit_parameter();
    uint16_t address = HL;
    uint8_t value = read_memory(address);
    write_register_8_bit(destination_register, value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_hl_ind_r() {
    uint8_t source_register = read_second_register_8_bit_parameter();
    uint16_t address = HL;
    uint8_t value = read_register_8_bit(source_register);
    write_memory(address, value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_hl_ind_imm() {
    uint16_t address = HL;
    uint8_t value = fetchOpcode();
    write_memory(address, value);
    return 12; // 12 cycles
}

void GameBoyEmulator::op_ld_a_bc_ind() {
    uint16_t address = BC;
    uint8_t value = read_memory(address);
    setA(value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_a_de_ind() {
    uint16_t address = DE;
    uint8_t value = read_memory(address);
    setA(value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_bc_ind_a() {
    uint16_t address = BC;
    uint8_t value = getA();
    write_memory(address, value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_de_ind_a() {
    uint16_t address = DE;
    uint8_t value = getA();
    write_memory(address, value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_a_imm_ind() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    uint8_t value = read_memory(address);
    setA(value);
    return 16; // 16 cycles
}

void GameBoyEmulator::op_ld_imm_ind_a() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    uint8_t value = getA();
    write_memory(address, value);
    return 16; // 16 cycles
}

void GameBoyEmulator::op_ldh_a_c_ind() {
    uint16_t address = 0xFF00 + getC();
    uint8_t value = read_memory(address);
    setA(value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ldh_c_ind_a() {
    uint16_t address = 0xFF00 + getC();
    uint8_t value = getA();
    write_memory(address, value);
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ldh_a_imm_ind() {
    uint16_t address = 0xFF00 + fetchOpcode();
    uint8_t value = read_memory(address);
    setA(value);
    return 12; // 12 cycles
}

void GameBoyEmulator::op_ldh_imm_ind_a() {
    uint8_t value = getA();
    uint16_t address = 0xFF00 + fetchOpcode();
    write_memory(address, value);
    return 12; // 12 cycles
}

void GameBoyEmulator::op_ld_a_hl_ind_dec() {
    uint16_t address = HL;
    uint8_t value = read_memory(address);
    setA(value);
    HL--;
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_hl_ind_dec_a() {
    uint16_t address = HL;
    uint8_t value = getA();
    write_memory(address, value);
    HL--;
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_a_hl_ind_inc() {
    uint16_t address = HL;
    uint8_t value = read_memory(address);
    setA(value);
    HL++;
    return 8; // 8 cycles
}

void GameBoyEmulator::op_ld_hl_ind_inc_a() {
    uint16_t address = HL;
    uint8_t value = getA();
    write_memory(address, value);
    HL++;
    return 8; // 8 cycles
}

// 16-bit load instructions
void GameBoyEmulator::op_ld_rr_imm() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = endian_swap(fetchOpcode(), fetchOpcode());
    write_register_16_bit(register_number, value);
    return 12; // 12 cycles
}

void GameBoyEmulator::op_ld_imm_ind_sp() {
    uint16_t address = endian_swap(fetchOpcode(), fetchOpcode());
    write_memory(address, SP & 0xFF);
    write_memory(address + 1, SP >> 8);
    return 16; // 16 cycles
}

void GameBoyEmulator::op_ld_sp_hl() {
    SP = HL;
    return 8; // 8 cycles
}

void GameBoyEmulator::op_push_rr() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = read_register_16_bit_stack(register_number);
    write_memory(SP - 1, value & 0xFF);
    write_memory(SP - 2, value >> 8);
    SP -= 2;
    return 16; // 16 cycles
}

void GameBoyEmulator::op_pop_rr() {
    uint8_t register_number = read_first_register_16_bit_parameter();
    uint16_t value = endian_swap(read_memory(SP), read_memory(SP + 1));
    write_register_16_bit_stack(register_number, value);
    SP += 2;
    return 12; // 12 cycles
}

void GameBoyEmulator::op_ld_hl_sp_e() {
    // uint8_t value = fetchOpcode();
    // HL = SP + value;
    // setFlagZ(0);
    // setFlagN(0);
    // setFlagH((SP & 0x0F) + (value & 0x0F) > 0x0F);
    // setFlagC((SP & 0xFF) + (value & 0xFF) > 0xFF);
    // return 12; // 12 cycles
}

// 8-bit arithmetic and logical instructions
void GameBoyEmulator::op_add_r() {
    // TODO
}

void GameBoyEmulator::op_adc_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_add_imm() {
    // TODO
}

void GameBoyEmulator::op_adc_r() {
    // TODO
}

void GameBoyEmulator::op_adc_imm() {
    // TODO
}

void GameBoyEmulator::op_sub_r() {
    // TODO
}

void GameBoyEmulator::op_sub_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_sub_imm() {
    // TODO
}

void GameBoyEmulator::op_sbc_r() {
    // TODO
}

void GameBoyEmulator::op_sbc_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_sbc_imm() {
    // TODO
}

void GameBoyEmulator::op_cp_r() {
    // TODO
}

void GameBoyEmulator::op_inc_r() {
    // TODO
}

void GameBoyEmulator::op_inc_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_dec_r() {
    // TODO
}

void GameBoyEmulator::op_dec_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_and_r() {
    // TODO
}

void GameBoyEmulator::op_and_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_and_imm() {
    // TODO
}

void GameBoyEmulator::op_or_r() {
    // TODO
}

void GameBoyEmulator::op_or_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_or_imm() {
    // TODO
}

void GameBoyEmulator::op_xor_r() {
    // TODO
}

void GameBoyEmulator::op_xor_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_xor_imm() {
    // TODO
}

void GameBoyEmulator::op_ccf() {
    // TODO
}

void GameBoyEmulator::op_scf() {
    // TODO
}

void GameBoyEmulator::op_daa() {
    // TODO
}

void GameBoyEmulator::op_cpl() {
    // TODO
}

// 16-bit arithmetic instructions
void GameBoyEmulator::op_inc_rr() {
    // TODO
}

void GameBoyEmulator::op_dec_rr() {
    // TODO
}

void GameBoyEmulator::op_add_hl_rr() {
    // TODO
}

void GameBoyEmulator::op_add_sp_e() {
    // TODO
}

// Rotate, shift, and bit operation instructions
void GameBoyEmulator::op_rlca() {
    // TODO
}

void GameBoyEmulator::op_rrca() {
    // TODO
}

void GameBoyEmulator::op_rla() {
    // TODO
}

void GameBoyEmulator::op_rra() {
    // TODO
}

// Control flow instructions
void GameBoyEmulator::op_jp_imm() {
    // TODO
}

void GameBoyEmulator::op_jp_hl() {
    // TODO
}

void GameBoyEmulator::op_jp_cc_imm() {
    // TODO
}

void GameBoyEmulator::op_jr_e() {
    // TODO
}

void GameBoyEmulator::op_jr_cc_e() {
    // TODO
}

void GameBoyEmulator::op_call_imm() {
    // TODO
}

void GameBoyEmulator::op_call_cc_imm() {
    // TODO
}

void GameBoyEmulator::op_ret() {
    // TODO
}

void GameBoyEmulator::op_ret_cc() {
    // TODO
}

void GameBoyEmulator::op_reti() {
    // TODO
}

void GameBoyEmulator::op_rst_imm() {
    // TODO
}

// Miscellaneous instructions
void GameBoyEmulator::op_halt() {
    // TODO
}

void GameBoyEmulator::op_stop() {
    // TODO
}

void GameBoyEmulator::op_di() {
    // TODO
}

void GameBoyEmulator::op_ei() {
    // TODO
}

void GameBoyEmulator::op_nop() {
    // TODO
}

// CB prefix instructions
void GameBoyEmulator::op_rlc_r() {
    // TODO
}

void GameBoyEmulator::op_rlc_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_rrc_r() {
    // TODO
}

void GameBoyEmulator::op_rrc_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_rl_r() {
    // TODO
}

void GameBoyEmulator::op_rl_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_rr_r() {
    // TODO
}

void GameBoyEmulator::op_rr_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_sla_r() {
    // TODO
}

void GameBoyEmulator::op_sla_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_sra_r() {
    // TODO
}

void GameBoyEmulator::op_sra_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_swap_r() {
    // TODO
}

void GameBoyEmulator::op_swap_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_srl_r() {
    // TODO
}

void GameBoyEmulator::op_srl_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_bit_b_r() {
    // TODO
}

void GameBoyEmulator::op_bit_b_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_res_b_r() {
    // TODO
}

void GameBoyEmulator::op_res_b_hl_ind() {
    // TODO
}

void GameBoyEmulator::op_set_b_r() {
    // TODO
}

void GameBoyEmulator::op_set_b_hl_ind() {
    // TODO
}
