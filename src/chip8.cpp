#include "chip8.h"

unsigned char chip8_characters[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

Chip8::Chip8() {}
Chip8::~Chip8() {}

void Chip8::initialize() {
    // Instructions start at address 512 or 0x200
    pc = 0x200;

    I = 0;
    sp = 0;

    delay_timer = 0;
    sound_timer = 0;

    // Load characters at lowest part of memory
    for (int i = 0; i < 80; i++) {
        memory[i] = chip8_characters[i];
    }

    for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
        display[i] = 0;
    }

    for (int i = 0; i < 16; i++) {
        keypad[i] = 0;
        registers[i] = 0;
        stack[i] = 0; 
    }

    srand(time(NULL));
}

int Chip8::get_file_size(char* file_path) {
    std::ifstream input(file_path, std::ios::binary);

    input.seekg(0, std::ios::end);
    return input.tellg();
}

bool Chip8::load_rom(char* rom_path) {
    initialize();
    int rom_size = get_file_size(rom_path);
    std::ifstream input(rom_path, std::ios::binary);

    if (!input) {
        std::cout << "Failed to read ROM" << std::endl;
        return false;
    }

    char *buffer = new char[rom_size];

    input.read(buffer, rom_size);

    if (rom_size >= MEMORY_SIZE - INTERPRETER_SPACE) {
        std::cout << "ROM is too large" << std::endl;
        return false;
    } else {
        for (int i = 0; i < rom_size; i++) {
            memory[INTERPRETER_SPACE + i] = buffer[i];
        }
    }

    delete[] buffer;
    input.close();

    return true;
}

void Chip8::execute_instruction() {
    uint16_t opcode = memory[pc] << 8 | memory[pc + 1];

    // Only look at highest 4 bits as we can't match exact opcode parameters
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0:
                    // 00E0: Clear the display
                    for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) {
                        display[i] = 0;
                    }
                    pc += 2;
                    break;

                case 0x00EE:
                    // 00EE: Return from a routine
                    pc = stack[sp];
                    sp--;
                    pc += 2;
                    break;
                
                default:
                    // 0nnn: Execute routine at nnn
                    pc = opcode;
                    break;

            }
            break;

        case 0x1000:
            // 1nnn: Jump to address nnn
            pc = opcode & 0x0FFF;
            break;

        case 0x2000:
            // 2nnn: Call routine pointed at by nnn
            sp++;
            stack[sp] = pc;
            pc = opcode & 0x0FFF;
            break;

        case 0x3000:
            // 3xkk: Skip next instruction if register x is equal to lowest 8 bits
            if (registers[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

        case 0x4000:
            // 4xkk: Skip next instruction if register x is not equal to lowest 8 bits
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
            pc += 2;
            break;

        case 0x7000:
            // 7xkk: Add kk to register x
            registers[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
            pc += 2;
            break;

        case 0x8000: {
            // Iterate over possibilities starting with 8
            uint8_t x = (opcode & 0x0F00) >> 8;
            uint8_t y = (opcode & 0x00F0) >> 4;

            // Only look at lowest 4 bits to distinguish between instructions
            switch (opcode & 0x000F) {
                case 0:
                    // 8xy0: Store value of register y in register x
                    registers[x] = registers[y];
                    pc += 2;
                    break;

                case 1:
                    // 8xy1: Register x is assigned register x OR register y
                    registers[x] = registers[x] | registers[y];
                    pc += 2;
                    break;

                case 2:
                    // 8xy2: Register x is assigned register x AND register y
                    registers[x] = registers[x] & registers[y];
                    pc += 2;
                    break;

                case 3:
                    // 8xy3: Register x is assigned register x XOR register y
                    registers[x] = registers[x] ^ registers[y];
                    pc += 2;
                    break;

                case 4:
                    // 8xy4: Register x is assigned register x + register y, carry goes to register F
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
                    // 8xy5: Register x is assigned register x - register y, carry goes to register F
                    if (registers[x] > registers[y]) {
                        registers[15] = 0x1;
                    } else {
                        registers[15] = 0x0;
                    }
                    registers[x] -= registers[y];
                    pc += 2;
                    break;

                case 6:
                    // 8xy6: Register F is assigned LSB of register x; register x divided by 2
                    registers[15] = registers[x] & 0x01;
                    registers[x] = registers[x] >> 1;
                    pc += 2;
                    break;

                case 7:
                    // 8xy7: Register x is assigned register y - register x; register F = NOT borrow
                    if (registers[y] > registers[x]) {
                        registers[15] = 0x1;
                    } else {
                        registers[15] = 0x0;
                    }

                    registers[x] = registers[x] - registers[y];
                    pc += 2;
                    break;

                case 14:
                    // 8xyE: Set register F to MSB of register x; multiply register x by 2
                    registers[15] = registers[x] >> 7;
                    registers[x] = registers[x] << 1;
                    pc += 2;
                    break;
            }       
            break;
        }
        case 0x9000:
            // 9xy0: Skip next instruction is register x != register y
            if (registers[(opcode & 0x0F00) >> 8] == registers[(opcode & 0x00F0) >> 4]) {
                pc += 2;
            } else {
                pc += 4;
            }
            break;

        case 0xA000:
            // Annn: Assign nnn to I
            I = opcode & 0x0FFF;
            pc += 2;
            break;
        
        case 0xB000:
            // Bnnn: Jump to nnn + register 0
            pc = (opcode & 0x0FFF) + registers[0];
            break;
        
        case 0xC000:
            // Cxkk: Assign random byte AND kk to register x
            registers[(opcode & 0x0F00) >> 8] = (rand() % 256) & (opcode & 0x00FF);
            pc += 2;
            break;

        case 0xD000: {
            // Dxyn: Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
            uint8_t x = registers[(opcode & 0x0F00) >> 8];
            uint8_t y = registers[(opcode & 0x00F0) >> 4];
            uint8_t n = opcode & 0x000F;

            registers[15] = 0;

            for (int i = 0; i < n; i++) {
                uint8_t pixel_value = memory[I + i];

                for (int j = 0; j < 8; j++) {
                    
                    if ((pixel_value & (0x80 >> j)) != 0) {
                        if(display[(x + j + ((y + i) * DISPLAY_WIDTH))] == 1) {
                            registers[15] = 1;
                        }

                        display[x + j + ((y + i) * DISPLAY_WIDTH)] ^= 1;
                    }
                } 
            }

            pc += 2;
            draw_flag = true;
            break;
        }

        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x009E:
                    // Ex9E: Skip next instruction if key with value x is pressed
                    if (keypad[(opcode & 0x0F00) >> 8] == 1) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
                
                case 0x00A1:
                    // ExA1: Skip next instruction if key with value x is not pressed
                    if (keypad[(opcode & 0x0F00) >> 8] != 1) {
                        pc += 4;
                    } else {
                        pc += 2;
                    }
                    break;
            }
            break;

        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x0007:
                    // Fx07: assign value of delay timer to register x
                    registers[(opcode & 0x0F00) >> 8] = delay_timer;
                    pc += 2;
                    break;
                
                case 0x000A: {
                    // Fx0A: Wait for key press and store key value in register x
                    bool key_pressed = false;

                    while (!key_pressed) {
                        uint8_t key = 0;
                        while (key < 16) {
                            if (keypad[key] == 1) {
                                registers[(opcode & 0x0F00) >> 8] = key;
                                key_pressed = true;
                                key = 16;
                            } else {
                                key++;
                            }
                        }
                        
                    }

                    pc += 2;
                    break;
                }

                case 0x0015:
                    // Fx15: Assign value of register x to delay timer
                    delay_timer = registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x0018:
                    // Fx18: Assign value of register x to sound timer
                    sound_timer = registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;

                case 0x001E:
                    // Fx1E: Add value of register x to I
                    I += registers[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                
                case 0x0029:
                    // Fx29: assign location of sprite for register x to I
                    I = registers[(opcode & 0x0F00) >> 8] * 0x5;
                    pc += 2;
                    break;

                case 0x0033: {
                    // Fx33: Store BCD-representations of val
                    uint8_t value = registers[(opcode & 0x0F00) >> 8];
                    memory[I] = value / 100;
                    memory[I + 1] = (value / 10) % 10;
                    memory[I + 2] = value % 10;

                    pc += 2;
                    break;
                }
                
                case 0x0055:
                    // Fx55: Store registers 0 to x in memory starting at I
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        memory[I + i] = registers[i];
                    }

                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

                case 0x0065:
                    // Fx65: Read registers 0 to x from memory starting at I
                    for (int i = 0; i <= ((opcode & 0x0F00) >> 8); i++) {
                        registers[i] = memory[I + i];
                    }

                    I += ((opcode & 0x0F00) >> 8) + 1;
                    pc += 2;
                    break;

            }
            break;
        
        default:
            std::cout << "Invalid opcode detected" << std::endl;
            exit(1);
    }

    if (delay_timer > 0) {
        --delay_timer;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            // TODO: Implement sound
        }
        sound_timer--;
    }
}

