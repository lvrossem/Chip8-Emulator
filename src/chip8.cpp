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

    // Only look at highest 8 bits as we can't match exact opcode parameters
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

        case 0x2000:
            // Call routine pointed at by lowest 12 bits
            sp++;
            stack[sp] = pc;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000:
            // Skip next instruction if register x is equal to lowest 8 bits
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        case 0x4000:
            // Skip next instruction if register x is not equal to lowest 8 bits
            if (registers[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        case 0x5000:
            // 5xy0: Skip next instruction if registers x and y are equal
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        case 0x6000:
            // 6xkk: Put kk in register x
            registers[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
            break;

        case 0x7000:
            // 7xkk: add kk to register x
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            break;

        case 0x8000:
            // Iterate over possibilities starting with 8
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            switch (opcode & 0x000F) {
                case 0:
                    // 8xy0: store value of register y in register x
                    registers[x] = registers[y];
                    break;
                case 1:
                    // 8xy1: register x is assigned register x OR register y
                    registers[x] = registers[x] | registers[y];
                    break;
                case 2:
                    // 8xy2: register x is assigned register x AND register y
                    registers[x] = registers[x] & registers[y];
                    break;

                case 3:
                    // 8xy3: register x is assigned register x XOR register y
                    registers[x] = registers[x] ^ registers[y];
                    break;

                case 4:
                    // 8xy4: register x is assigned register x + register y, carry goes to register F
                    registers[x] += registers[y];

                    if (0xFF - registers[x] < registers[y]) {
                        // There is carry
                        registers[15] = 0x1;
                    } else {
                        registers[15] = 0x0;
                    }
                    pc += 2;
                    break;
                case 5:
                    // 8xy5: register x is assigned register x - register y, carry goes to register F
                    if (registers[x] > registers[y]) {
                        registers[15] = 0x1;
                    } else {
                        registers[15] = 0x0;
                    }
                    registers[x] -= registers[y];
                    break;

                case 6:
                    // 8xy6: register F is assigned LSB of register x; register x divided by 2
                    registers[15] = registers[x] & 0x0001;
                    registers[x] = registers[x] >> 1;
                    break;

                case 7:
                    // 8xy7: register x is assigned register y - register x; register F = NOT borrow
                    if (registers[y] > registers[x]) {
                        registers[15] = 0x1;
                    } else {
                        registers[15] = 0x0;
                    }

                    registers[x] = registers[x] - registers[y];

            }           
    }       
}

