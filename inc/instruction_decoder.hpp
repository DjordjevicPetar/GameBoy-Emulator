#ifndef INSTRUCTION_DECODER_H_
#define INSTRUCTION_DECODER_H_

#include <cstdint>

class GameBoyEmulator;

class InstructionDecoder {
public:
    // Op struct for instruction pattern matching
    struct Op {
        uint8_t mask;
        uint8_t pattern;

        Op(uint8_t mask, uint8_t pattern) {
            this->mask = mask;
            this->pattern = pattern;
        }

        bool operator==(const Op& op_second) const {
            return this->mask == op_second.mask && this->pattern == op_second.pattern;
        }
    };

    // Hash function for Op struct (for use in unordered_map)
    struct OpHash {
        size_t operator()(const Op &op) const {
            return (size_t(op.mask) << 8) | op.pattern;
        }
    };

    // Initialize and register all instruction handlers
    static void initializeHandlers();
    
private:
    // Helper method to register individual instructions
    static void registerInstructions();
    // Helper method to register CB prefix instructions
    static void registerCbInstructions();
};

#endif
