#ifndef CHIP8
#define CHIP8

#define MEMORY_SIZE 4096
#define INTERPRETER_SPACE 512
#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH 64

#include <random>
#include <stdint.h>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "time.h"

class Chip8 {
    private:
        bool draw_flag;

        uint8_t memory[MEMORY_SIZE];
        uint8_t keypad[16];
        uint8_t registers[16];
        uint8_t sound_timer;
        uint8_t delay_timer;
        uint8_t sp;

        uint16_t stack[16];
        uint16_t I;
        uint16_t pc;

    public:
        Chip8();
        ~Chip8();

        uint8_t* get_keypad() { return keypad; }
        bool get_draw_flag() { return draw_flag; }
        void set_draw_flag(bool flag) { draw_flag = flag; }

        void initialize();
        void execute_instruction();

        int get_file_size(char* file_path);
        bool load_rom(char* rom_path);

        uint8_t display[DISPLAY_HEIGHT * DISPLAY_WIDTH];
};

#endif