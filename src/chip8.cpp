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

int Chip8::get_file_size(char* file_path) {
    std::ifstream input(file_path, std::ios::binary);

    input.seekg(0, std::ios::end);
    return input.tellg();
}

bool Chip8::load_rom(char* rom_path) {
    std::ifstream input(rom_path, std::ios::binary);

    if (!input) {
        std::cout << "Failed to read ROM" << std::endl;
        return false;
    }

    int rom_size = get_file_size(rom_path);

    char *buffer = new char[rom_size];

    input.read(buffer, rom_size);

    if (rom_size >= MEMORY_SIZE - INTERPRETER_SPACE) {
        std::cout << "ROM is too large" << std::endl;
        return false;
    } else {
        for (int i = 0; i = rom_size; i++) {
            memory[INTERPRETER_SPACE + i] = buffer[i];
        }
    }

    delete[] buffer;
    input.close();

    return true;
}

void Chip8::execute_instruction() {
    uint16_t opcode = memory[pc] << 8 | memory[pc + 1];

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x0E00:
                    // Clear the display
                    for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) {
                        display[i] = 0;
                    }
                    pc += 2;
                    break;

                case 0x00EE:
                    // Return from a routine
                    pc = stack[sp];
                    sp--;
                    pc += 2;
                    break;
            }
            break;

        case 0x1000:
            // Jump to address pointed at by lowest 12 bits of opcode
            pc = opcode & 0x0FFF;
            break;
    }
}

