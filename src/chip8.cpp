#include "chip8.h"

Chip8::Chip8() {}
Chip8::~Chip8() {}

void Chip8::initialize() {
    // Instructions start at address 512 or 0x200
    pc = 0x200;

    I = 0;
    sp = 0;

    delay_timer = 0;
    sound_timer = 0;

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        display[i] = 0;
    }

    for (int i = 0; i < 16; i++) {
        keypad[i] = 0;
        registers[i] = 0;
        stack[i] = 0; 
    }
}

bool Chip8::load_rom(char* rom_path) {
    std::ifstream input(rom_path, std::ios::binary);

    if (!input) {
        std::cout << "Failed to read ROM" << std::endl;
        return false;
    }

    input.seekg(0, std::ios::end);
    int rom_size = input.tellg();

    char *buffer = new char[rom_size];

    input.read(buffer, rom_size);

    if (rom_size < MEMORY_SIZE - INTERPRETER_SPACE) {
        for (int i = 0; i = rom_size; i++) {

        }
    } else {
        std::cout << "ROM is too large" << std::endl;
    }

    input.close();
}