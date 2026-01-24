#include "game_boy_emulator.hpp"
#include "memory/cartridge.hpp"
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
        u8 cycles = cpu_.execute_next_instruction();
        cycles += cpu_.handle_interrupts();
        cycles_executed_ += cycles;
        
        // Handle timer
        timer_.update_timer(cycles);

        // Handle Graphics
        PPUMode prev_mode = ppu_.get_mode();
        u8 prev_ly = ppu_.get_ly();

        ppu_.step(cycles);

        if (ppu_.get_ly() == 0 && ppu_.get_mode() == PPUMode::OAM) {
            if (prev_ly != 0 || prev_mode != PPUMode::OAM) {
                next_frame_detected = true;
            }
        }

        instruction_count++;
        if (instruction_count > 100) {
            if (check_quit_request()) return !stop_cpu_;
        }
        // TODO: Audio, etc.
    }
    return !stop_cpu_;
}

void GameBoyEmulator::emulate() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_Window* window = SDL_CreateWindow("Game Boy Emulator", LCD_WIDTH * 4, LCD_HEIGHT * 4, SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 160, 144);
    if (!texture) {
        std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    bool running = true;
    SDL_Event e;

    const double target_fps = 59.71;
    const double target_ms = 1000.0 / target_fps;

    u32 last = SDL_GetTicks();

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

        if (SDL_LockTexture(texture, NULL, &pixels, &pitch)) {  // SDL3 returns true on success
            const std::vector<std::vector<u32>>& fb = ppu_.get_framebuffer();

            for (int y = 0; y < LCD_HEIGHT; y++) {
                u32* dst = reinterpret_cast<u32*>(static_cast<u8*>(pixels) + y * pitch);

                for (int x = 0; x < LCD_WIDTH; x++) {
                    dst[x] = fb[y][x];
                }
            }

            SDL_UnlockTexture(texture);
        }
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        u64 now = SDL_GetTicks();
        double elapsed = (double)(now - last);

        if (elapsed < target_ms) {
            SDL_Delay((u32)target_ms - elapsed);
        }

        last = SDL_GetTicks();
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
