#include "../inc/game_boy_emulator.hpp"
#include "../inc/cartridge.hpp"
#include <iostream>

GameBoyEmulator* GameBoyEmulator::instance_ = nullptr;

std::string GameBoyEmulator::filepath_ = "";

GameBoyEmulator::GameBoyEmulator() 
    : interrupt_controller_()
    , ppu_(&interrupt_controller_)
    , timer_(&interrupt_controller_)
    , mmu_(filepath_, &ppu_, &timer_, &interrupt_controller_)
    , cpu_(&mmu_, &interrupt_controller_)
     {}

GameBoyEmulator* GameBoyEmulator::getInstance() {
    if (instance_ == nullptr) {
        instance_ = new GameBoyEmulator();
    }
    return instance_;
}

void GameBoyEmulator::setFilepath(const std::string& filepath) {
    filepath_ = filepath;
}

bool GameBoyEmulator::check_quit_request() {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT || e.key.scancode == SDL_SCANCODE_ESCAPE) {
            stop_cpu_ = true;
            return true;
        }
    }
    return false;
}

bool GameBoyEmulator::run_until_next_frame() {
    bool next_frame_detected = false;
    int instruction_count = 0;

    while (!next_frame_detected && !stop_cpu_) {
        uint8_t cycles = cpu_.execute_next_instruction();
        cycles += cpu_.handle_interrupts();
        cycles_executed_ += cycles;
        
        // Handle timer
        timer_.update_timer(cycles);

        // Handle Graphics
        ppu_.step(cycles);

        if (ppu_.get_ly() == 0 && ppu_.get_mode() == PPUMode::OAM) {
            std::cout << "Next frame detected!\n";
            next_frame_detected = true;
        }

        instruction_count++;
        if (instruction_count > 100) {
            if (check_quit_request()) return !stop_cpu_;
        }

        std::cout << "Stop_cpu = " << stop_cpu_ << '\n';
        // TODO: Audio, etc.
    }
    return !stop_cpu_;
}

void GameBoyEmulator::emulate() {
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window* window = SDL_CreateWindow("Game Boy Emulator", LCD_WIDTH * 4, LCD_HEIGHT * 4, SDL_WINDOW_RESIZABLE);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);

    bool running = true;
    SDL_Event e;

    const double target_fps = 30;
    const double target_ms = 1000.0 / target_fps;

    uint32_t last = SDL_GetTicks();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (e.type == SDL_EVENT_KEY_DOWN && e.key.scancode == SDL_SCANCODE_ESCAPE) {
                running = false;
            }
        }
        
        if (!run_until_next_frame()) break;

        void* pixels;
        int pitch;

        if (SDL_LockTexture(texture, NULL, &pixels, &pitch) == 0) {
            const std::vector<std::vector<uint32_t>>& fb = ppu_.get_framebuffer();

            for (int y = 0; y < LCD_HEIGHT; y++) {
                uint32_t* dst = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pixels) + y * pitch);

                for (int x = 0; x < LCD_WIDTH; x++) {
                    dst[x] = fb[y][x];
                }
            }

            SDL_UnlockTexture(texture);
        }
        std::cout << "Rendering...\n";
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        uint64_t now = SDL_GetTicks();
        double elapsed = (double)(now - last);

        if (elapsed < target_ms) {
            SDL_Delay((uint32_t)target_ms - elapsed);
        }

        last = SDL_GetTicks();
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
