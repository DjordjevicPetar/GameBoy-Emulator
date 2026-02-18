#include "cartridge.hpp"

Cartridge::Cartridge(std::string path) {
    load_rom(path);
}

bool Cartridge::load_rom(std::string path) {
    std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error: could not open ROM";
        return false;
    }
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<u8> buffer = std::vector<u8>(fileSize);
    file.read(reinterpret_cast<char *>(buffer.data()), fileSize);
    for (int i = 0; i < fileSize; i++) {
        rom.push_back(buffer[i]);
    }
    parse_header();
    return true;
}

void Cartridge::parse_header() {
    rom_banks = rom.size() / SWITCHABLE_ROM_SIZE;

    u8 val = rom[HEADER_RAM_SIZE_ADDR];
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
            mbc = std::make_unique<MBC0>(rom, ram);
            break;

        case 0x01: case 0x02: case 0x03:
            mbc = std::make_unique<MBC1>(rom, ram);
            break;

        // TODO: MBC2 (0x05-0x06), MBC3 (0x0F-0x13, includes RTC), and MBC5 (0x19-0x1E)
        // cover the vast majority of the remaining Game Boy library. MBC3 is needed
        // for Pokemon Gold/Silver/Crystal, MBC5 for Pokemon Yellow and many later titles.
        default:
            std::cerr << "Error: Unsupported cartridge type 0x" 
                      << std::hex << (int)cartridge_type << std::dec 
                      << ". Only MBC0 (0x00) and MBC1 (0x01-0x03) are supported." << std::endl;
            mbc = std::make_unique<MBC0>(rom, ram);
            break;
    }

}

void Cartridge::print_rom() {
    for (size_t i = 0; i < rom.size(); i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)rom[i];
    }
    std::cout << std::dec;
}


u8 Cartridge::read8(u16 addr) const {
    return mbc->read(addr);
}

void Cartridge::write8(u16 addr, u8 val) {
    mbc->write(addr, val);
}