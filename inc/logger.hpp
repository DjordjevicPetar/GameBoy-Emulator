#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include <fstream>
#include <string>
#include <cstdint>

class Logger {
private:
    static bool enabled;
    static std::ofstream log_file;

public:
    static void init(bool enable, const std::string& filename = "cpu_log.txt");
    static void close();
    static bool isEnabled();
    static void log(const std::string& func_name, uint8_t opcode,
                    uint16_t AF, uint16_t BC, uint16_t DE, uint16_t HL,
                    uint16_t SP, uint16_t PC, bool IME,
                    const std::string& details = "");
};

#endif

