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

    static void log(u8 A, u8 F, u8 B, u8 C, 
                    u8 D, u8 E, u8 H, u8 L,
                    u16 SP, u16 PC,
                    u8 mem0, u8 mem1, u8 mem2, u8 mem3);
};
