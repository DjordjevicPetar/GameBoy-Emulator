#include "logger.hpp"
#include <iomanip>

// Static member definitions
bool Logger::enabled = false;
std::ofstream Logger::log_file;

void Logger::init(bool enable, const std::string& filename) {
    enabled = enable;
    if (enabled) {
        log_file.open(filename, std::ios::out | std::ios::trunc);  // Overwrite existing file
        // No header - clean format for diff comparison
    }
}

void Logger::close() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

bool Logger::isEnabled() {
    return enabled;
}

void Logger::log(u8 A, u8 F, u8 B, u8 C,
                 u8 D, u8 E, u8 H, u8 L,
                 u16 SP, u16 PC,
                 u8 mem0, u8 mem1, u8 mem2, u8 mem3) {
    if (!enabled || !log_file.is_open()) return;

    // TODO: "PC: 00:XXXX" hardcodes ROM bank 00. For MBC-banked ROMs, PC in
    // 0x4000-0x7FFF should show the current ROM bank number, not always 00.
    log_file << std::hex << std::uppercase << std::setfill('0');
    log_file << "A: " << std::setw(2) << (int)A
             << " F: " << std::setw(2) << (int)F
             << " B: " << std::setw(2) << (int)B
             << " C: " << std::setw(2) << (int)C
             << " D: " << std::setw(2) << (int)D
             << " E: " << std::setw(2) << (int)E
             << " H: " << std::setw(2) << (int)H
             << " L: " << std::setw(2) << (int)L
             << " SP: " << std::setw(4) << SP
             << " PC: 00:" << std::setw(4) << PC
             << " (" << std::setw(2) << (int)mem0
             << " " << std::setw(2) << (int)mem1
             << " " << std::setw(2) << (int)mem2
             << " " << std::setw(2) << (int)mem3
             << ")" << std::endl;
}
