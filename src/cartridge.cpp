#include "../inc/cartridge.hpp"

Cartridge::Cartridge() {
    current_bank = 1;
    ram = vector<uint8_t>(); // TODO: determine the size
}

bool Cartridge::load_rom(string path) {
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
        rom.push_back(buffer[i]);
    }
    return true;
}

void Cartridge::parse_header() {
    rom_banks = rom.size() / SWITCHABLE_ROM_SIZE;

    uint8_t val = rom[HEADER_RAM_SIZE_ADDR];
    ram_banks = 0;
    switch (val) {
        case 0x00:
            ram_banks = 0;
            break;
        case 0x01: // Unused
            ram_banks = 0;
            break;
        case 0x02:
            ram_banks = 1;
            break;
        case 0x03:
            ram_banks = 4;
            break;
        case 0x04:
            ram_banks = 16;
            break;
        case 0x05:
            ram_banks = 8;
            break;
    }
    if (ram_banks > 0) {
        ram.resize(ram_banks * SWITCHABLE_RAM_SIZE);
    }
}

void Cartridge::print_rom() {
    for (size_t i = 0; i < rom.size(); i++) {
        cout << hex << setw(2) << setfill('0') << (int)rom[i];
    }
    cout << dec;
}


uint8_t Cartridge::read8_rom(uint16_t addr) const {
    if (addr <= STATIC_ROM_END) {
        return rom[addr];
    }
    else if (addr <= SWITCHABLE_ROM_END) {
        return rom[current_bank * SWITCHABLE_ROM_SIZE + (addr - SWITCHABLE_ROM_START)];
    }
    return 0xff; // might change
}

void Cartridge::write8_rom(uint16_t addr, uint8_t val) {
    if (addr >= ROM_BANK_SELECT_START && addr <= ROM_BANK_SELECT_END) {
        current_bank = val & MBC1_ROM_BANKS_MASK;
        if (current_bank == 0) current_bank = 1;
        current_bank %= rom_banks;
    }
}

uint8_t Cartridge::read8_ram(uint16_t addr) const {
    return 0xff; // TODO
}

void Cartridge::write8_ram(uint16_t addr, uint8_t val) {
    // TODO
}