#ifndef EMULATOR_H_
#define EMULATOR_H_

class GameBoyEmulator {

private:

    static GameBoyEmulator* instance;
    std::string filepath;

    GameBoyEmulator();
    GameBoyEmulator(GameBoyEmulator&) = delete;
    GameBoyEmulator(GameBoyEmulator&&) = delete;

public:

    void emulate();
    void setFilepath(const std::string& filepath);
    GameBoyEmulator* getInstance();

};

#endif