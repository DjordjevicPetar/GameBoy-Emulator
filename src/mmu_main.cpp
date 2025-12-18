#include <iostream>
#include "../inc/cartridge.hpp"

using namespace std;

int main() {
    Cartridge cartridge;

    cartridge.loadROM("cpu_instrs.gb");
    cartridge.printROM();
    system("pause");

    

    return 0;
}