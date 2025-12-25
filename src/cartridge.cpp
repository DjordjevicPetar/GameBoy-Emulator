#include "../inc/cartridge.hpp"

Cartridge::Cartridge(std::string path) {
    load_rom(path);
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
    parse_header();
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

    cartridge_type = rom[0x0147];

    switch (cartridge_type) {
        case 0x00:
            mbc = make_unique<MBC0>(rom, ram);
            break;

        case 0x01: case 0x02: case 0x03:
            mbc = make_unique<MBC1>(rom, ram);
            break;
    }

}

void Cartridge::print_rom() {
    for (size_t i = 0; i < rom.size(); i++) {
        cout << hex << setw(2) << setfill('0') << (int)rom[i];
    }
    cout << dec;
}


uint8_t Cartridge::read8(uint16_t addr) const {
    return mbc->read(addr);
}

void Cartridge::write8(uint16_t addr, uint8_t val) {
    mbc->write(addr, val);
}