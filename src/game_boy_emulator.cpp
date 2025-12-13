#include "../inc/game_boy_emulator.h"

GameBoyEmulator* GameBoyEmulator::instance = nullptr;

GameBoyEmulator::GameBoyEmulator() {
    
}

GameBoyEmulator* GameBoyEmulator::getInstance() {

    if (instance == nullptr) {
        instance = new GameBoyEmulator();
    }

    return instance;
}

void GameBoyEmulator::setFilepath(const std::string& filepath) {
    this->filepath = filepath;
}