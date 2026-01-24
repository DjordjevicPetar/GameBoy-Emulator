#pragma once

#include "utils/types.hpp"


#include <cstddef>

// Forward declaration
class CPU;

class InstructionDecoder {
public:
    // Op struct for instruction pattern matching
    struct Op {
        u8 mask;
        u8 pattern;

        Op(u8 m, u8 p) : mask(m), pattern(p) {}

        bool operator==(const Op& other) const {
            return mask == other.mask && pattern == other.pattern;
        }
    };

    // Hash function for Op struct (for use in unordered_map)
    struct OpHash {
        std::size_t operator()(const Op& op) const {
            return (static_cast<std::size_t>(op.mask) << 8) | op.pattern;
        }
    };

    // Initialize and register all instruction handlers
    static void initializeHandlers(CPU* cpu);
    
private:
    static void registerInstructions(CPU* cpu);
    static void registerCbInstructions(CPU* cpu);
};

