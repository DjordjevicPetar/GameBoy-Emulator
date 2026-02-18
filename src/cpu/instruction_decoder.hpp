// TODO(refactor): This class is tightly coupled to CPU (friend access, CPU-specific
// function pointers) and has no state - it's just a bag of static functions.
// Option A: Make add_instruction a template utility and move registration into
//           CPU::initializeHandlers(). InstructionDecoder becomes a small generic
//           utility (or just a free function) with zero CPU dependency.
// Option B: Turn this into a namespace instead of a class.
// Either way, the mask/pattern registration calls belong in CPU since they reference
// CPU's own methods. add_instruction + find_first_zero are generic utilities.
#pragma once

#include "../utils/types.hpp"

#include <array>

class CPU;

using Handler = u8 (CPU::*)();

class InstructionDecoder {
public:
    static void initializeHandlers(CPU* cpu);
    
private:
    static void registerInstructions(CPU* cpu);
    static void registerCbInstructions(CPU* cpu);
    static void add_instruction(std::array<Handler, 256>& table, u8 mask, u8 pattern, Handler handler);
    static u8 find_first_zero(u8 value);
};

