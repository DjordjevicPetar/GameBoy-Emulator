#ifndef MAIN_H_
#define MAIN_H_

#include <iostream>
#include <thread>
#include "../inc/emulator.h"

int main(int argc, char* argv[]){

    if (argc != 2) {
        std::cout << "ERROR: Program to execute not given" << std::endl;
        return 1;
    }

    Emulator* emulator = Emulator::getInstance();

    emulator->setFilepath(argv[1]);

    std::thread runningProgram(&Emulator::emulate, emulator);

    runningProgram.join();

    return 0;

}

#endif