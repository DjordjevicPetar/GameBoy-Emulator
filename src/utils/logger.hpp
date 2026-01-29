#pragma once

#include "../utils/types.hpp"


#include <fstream>
#include <string>

class Logger {
private:
    static bool enabled;
    static std::ofstream log_file;

public:
    static void init(bool enable, const std::string& filename = "cpu_log.txt");
    static void close();
    static bool isEnabled();
    // Old format - commented out
    // static void log(const std::string& func_name, u8 opcode,
    //                 u16 AF, u16 BC, u16 DE, u16 HL,
    //                 u16 SP, u16 PC, bool IME,
    //                 u8 mem_pc, u8 mem_pc1,
    //                 const std::string& details = "");
    
    // EpicLog format: A: 01 F: B0 B: 00 C: 13 D: 00 E: D8 H: 01 L: 4D SP: FFFE PC: 00:0100 (00 C3 13 02)
    static void log(u8 A, u8 F, u8 B, u8 C, 
                    u8 D, u8 E, u8 H, u8 L,
                    u16 SP, u16 PC,
                    u8 mem0, u8 mem1, u8 mem2, u8 mem3);
};
