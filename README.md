# GameBoy-Emulator
A functional emulator of the original Nintendo Game Boy, written in C++. The project emulates core Game Boy hardware components including the CPU, PPU, APU, memory system, timers, interrupts and cartridge handling.

## Components:

### CPU emulation
- Full Game Boy opcode decoding
- CB-prefixed instruction support
- Arithmetic and logic operations
- Control flow and load instructions
- Interrupt Handling
### Graphics (PPU)
- Scanline-based rendering pipeline
- Background, window and sprite rendering
- LCD mode emulation
  - OAM Scan
  - Drawing
  - HBlank
  - VBlank
### Audio (APU)
- Square wave channels
- Wave channel
- Noise channel
### Memory System
- MMU implementation
- Cartridge loading
- MBC0 and MBC1 support
### Hardware emulation
- Joypad input support
- Hardware timer emulation
### Architecture
- Modular component-based design
- Separated hardware subsystems for maintainability and extensibility


## Tech Stack:
- C++
- Make
- SDL3
- Git / GitHub


## Build:
```bash
make build
```

## Run:
```bash
gameboy.exe <path-to-rom>
```

Example:

```bash
gameboy.exe roms/example.gb
```

## Project Structure

```text
audio/                  Audio Processing Unit
cpu/                    CPU core and instruction decoding
interrupt/              Interrupt controller
memory/                 MMU and cartridge handling
ppu/                    Graphics rendering pipeline
timer/                  Hardware timer emulation
utils/                  Helper classes and methods
game_boy_emulator.cpp   Main emulation loop
main.cpp

```

## Notes

- The PPU currently uses scanline/bulk rendering rather than fully cycle-accurate pixel rendering.
- The emulator focuses on functional accuracy and clean architecture