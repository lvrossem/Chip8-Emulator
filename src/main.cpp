#include <iostream>
#include <chrono>
#include <thread>
#include "stdint.h"
#include "SDL2/SDL.h"
#include "chip8.h"


// Keypad keymap
uint8_t keymap[16] = {
    SDLK_x,
    SDLK_1,
    SDLK_2,
    SDLK_3,
    SDLK_q,
    SDLK_w,
    SDLK_e,
    SDLK_a,
    SDLK_s,
    SDLK_d,
    SDLK_z,
    SDLK_c,
    SDLK_4,
    SDLK_r,
    SDLK_f,
    SDLK_v,
};

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "Please give path to ROM-file" << std::endl;
        return 1;
    }

    int screen_width = 1280;
    int screen_height = 640;

    SDL_Window *window;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Failed to initialize SDL" << std::endl;
        return 1;
    }

    window = SDL_CreateWindow("Chip-8", 
                               SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               screen_width,
                               screen_height,
                               SDL_WINDOW_SHOWN);

    if (!window) {
        std::cout << "Failed to create window" << std::endl;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);

    SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            64, 32);

    std::cout << "HERE 1\n";
    Chip8 chip8 = Chip8();
    uint32_t pixels[2048];

    load:
    // Attempt to load ROM
    if (!chip8.load_rom(argv[1])) {
        return 2;
    }

    // Emulation loop
    std::cout << "HERE 1\n";
    while (true) {
        chip8.execute_instruction();

        // Process SDL events
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) exit(0);

            // Process keydown events
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }

                if (e.key.keysym.sym == SDLK_F1) {
                    // Reset ROM
                    goto load;     
                }

                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.get_keypad()[i] = 1;
                    }
                }
            }
            // Process keyup events
            if (e.type == SDL_KEYUP) {
                for (int i = 0; i < 16; ++i) {
                    if (e.key.keysym.sym == keymap[i]) {
                        chip8.get_keypad()[i] = 0;
                    }
                }
            }
        }

        // If draw occurred, redraw SDL screen
        if (chip8.get_draw_flag()) {
            chip8.set_draw_flag(false);

            // Store pixels in temporary buffer
            for (int i = 0; i < DISPLAY_WIDTH * DISPLAY_HEIGHT; i++) {
                uint8_t pixel = chip8.get_display()[i];
                pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
            }

            // Update SDL rendering
            SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
            SDL_RenderPresent(renderer);
        }

        // Sleep to slow down emulation speed
        //std::this_thread::sleep_for(std::chrono::microseconds(1200));
    }
}