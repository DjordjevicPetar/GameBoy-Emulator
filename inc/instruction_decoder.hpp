#ifndef INSTRUCTION_DECODER_HPP_
#define INSTRUCTION_DECODER_HPP_

#include <cstddef>
#include <cstdint>

// Forward declaration
class CPU;

class InstructionDecoder {
public:
    // Op struct for instruction pattern matching
    struct Op {
        uint8_t mask;
        uint8_t pattern;

        Op(uint8_t m, uint8_t p) : mask(m), pattern(p) {}

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

#endif
