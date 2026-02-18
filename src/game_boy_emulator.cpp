#include "game_boy_emulator.hpp"
#include "memory/cartridge.hpp"
#include <iostream>

GameBoyEmulator* GameBoyEmulator::instance_ = nullptr;

std::string GameBoyEmulator::filepath_ = "";

GameBoyEmulator::GameBoyEmulator() 
    : interrupt_controller_()
    , joypad_(&interrupt_controller_)
    , apu_()
    , ppu_(&interrupt_controller_)
    , timer_(&interrupt_controller_)
    , mmu_(filepath_, &ppu_, &timer_, &interrupt_controller_, &joypad_, &apu_)
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

void GameBoyEmulator::check_events() { // returns true if 'QUIT'
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT || e.key.scancode == SDL_SCANCODE_ESCAPE) {
            stop_cpu_ = true;
        }
        if (e.type == SDL_EVENT_KEY_DOWN) {
            if (e.key.scancode == SDL_SCANCODE_Q) joypad_.press(Joypad::A);
            if (e.key.scancode == SDL_SCANCODE_W) joypad_.press(Joypad::B);
            if (e.key.scancode == SDL_SCANCODE_A) joypad_.press(Joypad::Select);
            if (e.key.scancode == SDL_SCANCODE_S) joypad_.press(Joypad::Start);

            if (e.key.scancode == SDL_SCANCODE_RIGHT) joypad_.press(Joypad::Right);
            if (e.key.scancode == SDL_SCANCODE_LEFT) joypad_.press(Joypad::Left);
            if (e.key.scancode == SDL_SCANCODE_UP) joypad_.press(Joypad::Up);
            if (e.key.scancode == SDL_SCANCODE_DOWN) joypad_.press(Joypad::Down);
        }
        if (e.type == SDL_EVENT_KEY_UP) {
            if (e.key.scancode == SDL_SCANCODE_Q) joypad_.release(Joypad::A);
            if (e.key.scancode == SDL_SCANCODE_W) joypad_.release(Joypad::B);
            if (e.key.scancode == SDL_SCANCODE_A) joypad_.release(Joypad::Select);
            if (e.key.scancode == SDL_SCANCODE_S) joypad_.release(Joypad::Start);

            if (e.key.scancode == SDL_SCANCODE_RIGHT) joypad_.release(Joypad::Right);
            if (e.key.scancode == SDL_SCANCODE_LEFT) joypad_.release(Joypad::Left);
            if (e.key.scancode == SDL_SCANCODE_UP) joypad_.release(Joypad::Up);
            if (e.key.scancode == SDL_SCANCODE_DOWN) joypad_.release(Joypad::Down);
        }
    }
}

void GameBoyEmulator::run_until_next_frame() {
    // TODO(cycle-accuracy): This loop steps the CPU one full instruction at a time,
    // then feeds ALL cycles to timer/PPU/APU at once. On real hardware, the CPU, PPU,
    // and timer all advance in lockstep every T-cycle (or at minimum every M-cycle = 4 T).
    // This means mid-instruction memory accesses (e.g., 16-bit writes during PUSH/CALL,
    // multi-byte reads during fetch_u16) should interleave with PPU/timer updates.
    // To pass blargg's mem_timing, mem_timing-2, and instr_timing tests, we need to
    // step components per M-cycle (4 T-cycles) instead of per instruction.
    // Approach: CPU returns cycles consumed per M-cycle step, and after each M-cycle
    // we tick timer, PPU, and APU by 4 T-cycles.
    bool next_frame_detected = false;
    int instruction_count = 0;

    while (!next_frame_detected && !stop_cpu_) {
        u8 cycles = cpu_.execute_next_instruction();
        // TODO(cycle-accuracy): Interrupt dispatch should happen BETWEEN instructions,
        // not be added to the previous instruction's cycles. The 20-cycle interrupt
        // dispatch (5 M-cycles: 2 wait + 2 push + 1 jump) should also tick PPU/timer
        // on each M-cycle, not all at once after the fact.
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
            check_events();
            if (stop_cpu_) return;
        }
        
        // Handle Audio
        apu_.step(cycles);
        cycles_from_audio_sample += cycles;

        while (cycles_from_audio_sample >= 87) {
            cycles_from_audio_sample -= 87;

            AudioSample sample = apu_.mix();

            audio_buffer.push_back(sample.left);
            audio_buffer.push_back(sample.right);
        }
    }
}

void GameBoyEmulator::emulate() {
    // Initialize SDL
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO)) {
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

    // Initialize audio
    SDL_AudioSpec spec;
    SDL_zero(spec);

    spec.freq = 48000;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;

    SDL_AudioStream* audio_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);

    if (!audio_stream) {
        std::cerr << "SDL_AudioStream failed: " << SDL_GetError() << std::endl;
    }

    SDL_ResumeAudioDevice(SDL_GetAudioStreamDevice(audio_stream));

    bool running = true;

    const double target_fps = 59.71;
    const double target_ms = 1000.0 / target_fps;

    u32 last = SDL_GetTicks();

    // TODO: Circular dependency (PPU needs MMU, MMU needs PPU) is resolved with a
    // post-construction setter. Consider restructuring so PPU reads VRAM/OAM directly
    // from its own memory, and only uses MMU for DMA source reads.
    ppu_.set_mmu(&mmu_);

    // Main emulation loop
    while (running) {
        check_events();
        
        run_until_next_frame();
        if (stop_cpu_) break;

        if (!audio_buffer.empty()) {
            SDL_PutAudioStreamData(
                audio_stream,
                audio_buffer.data(),
                audio_buffer.size() * sizeof(s16)
            );

            audio_buffer.clear();
        }

        unsigned long long queued = SDL_GetAudioStreamQueued(audio_stream);

        if (queued > 48000 * sizeof(s16)) {
            SDL_ClearAudioStream(audio_stream);
        }

        void* pixels;
        int pitch;

        if (SDL_LockTexture(texture, NULL, &pixels, &pitch)) {
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
