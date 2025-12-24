#include "../inc/logger.hpp"
#include <iomanip>

// Static member definitions
bool Logger::enabled = false;
std::ofstream Logger::log_file;

void Logger::init(bool enable, const std::string& filename) {
    enabled = enable;
    if (enabled) {
        log_file.open(filename);
        if (log_file.is_open()) {
            log_file << "=== GameBoy CPU Log ===" << std::endl;
            log_file << std::endl;
        }
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

void Logger::log(const std::string& func_name, uint8_t opcode,
                 uint16_t AF, uint16_t BC, uint16_t DE, uint16_t HL,
                 uint16_t SP, uint16_t PC, bool IME,
                 const std::string& details) {
    if (!enabled || !log_file.is_open()) return;

    log_file << std::hex << std::uppercase << std::setfill('0');
    log_file << "PC:" << std::setw(4) << PC 
             << " OP:" << std::setw(2) << (int)opcode
             << " | " << func_name;
    
    if (!details.empty()) {
        log_file << " [" << details << "]";
    }
    log_file << std::endl;
    
    log_file << "  AF:" << std::setw(4) << AF
             << " BC:" << std::setw(4) << BC
             << " DE:" << std::setw(4) << DE
             << " HL:" << std::setw(4) << HL
             << " SP:" << std::setw(4) << SP
             << " IME:" << (IME ? "1" : "0") << std::endl;
    log_file << std::dec;
}

