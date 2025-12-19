#include "../inc/cartridge.hpp"

Cartridge::Cartridge() {
    currentBank = 1;
    ram = vector<uint8_t>(); // TODO: determine the size
}

bool Cartridge::loadROM(string path) {
    ifstream file(path, ios::in | ios::binary | ios::ate);
    if (!file.is_open()) {
        cerr << "Error: could not open ROM";
        return false;
    }
    streampos fileSize = file.tellg();
    file.seekg(0, ios::beg);
    vector<uint8_t> buffer = vector<uint8_t>(fileSize);
    file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    for (int i = 0; i < fileSize; i++) {
        cout << hex << setw(2) << setfill('0') << (int)buffer[i];
    }
    for (int i = 0; i < fileSize; i++) {
        rom.push_back(buffer[i]);
    }
    system("pause");
    return true;
}

void Cartridge::printROM() {
    for (int i = 0; i < rom.size(); i++) {
        cout << hex << setw(2) << setfill('0') << (int)rom[i];
    }
    cout << dec;
    cout << 67;
}


uint8_t Cartridge::read8(uint16_t addr) {
    if (addr <= STATIC_ROM_END) {
        return rom[addr];
    }
    else if (addr <= SWITCHABLE_ROM_END) {
        return rom[currentBank * SWITCHABLE_ROM_SIZE + (addr - SWITCHABLE_ROM_START)];
    }
    return 0xff; // might change
}

void Cartridge::write8(uint16_t addr, uint8_t val) {
    if (addr >= BANK_SWITCHING_START && addr <= BANK_SWITCHING_END) {
        currentBank = val & ROM_BANKS_MASK;
        if (currentBank == 0) currentBank = 1;
    }
}