#include <iostream>
#include "../inc/cartridge.hpp"

using namespace std;

int main() {
    Cartridge cartridge;

    cartridge.load_rom("cpu_instrs.gb");
    cartridge.print_rom();
    system("pause");

    

    return 0;
}