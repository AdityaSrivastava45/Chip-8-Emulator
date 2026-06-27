/**
 * @file input.c
 * @brief SDL2 keyboard input — event processing and CHIP-8 key mapping.
 */

#include <SDL2/SDL.h>
#include "input.h"

/* ── Key Mapping ──────────────────────────────────────────────── */
/*
 * Map physical QWERTY keys to CHIP-8 hex keypad:
 * 1 2 3 4 -> 1 2 3 C
 * Q W E R -> 4 5 6 D
 * A S D F -> 7 8 9 E
 * Z X C V -> A 0 B F
 */
static uint8_t map_key(SDL_Scancode scancode, bool *matched) {
    *matched = true;
    switch (scancode) {
        case SDL_SCANCODE_1: return 0x1;
        case SDL_SCANCODE_2: return 0x2;
        case SDL_SCANCODE_3: return 0x3;
        case SDL_SCANCODE_4: return 0xC;
        
        case SDL_SCANCODE_Q: return 0x4;
        case SDL_SCANCODE_W: return 0x5;
        case SDL_SCANCODE_E: return 0x6;
        case SDL_SCANCODE_R: return 0xD;
        
        case SDL_SCANCODE_A: return 0x7;
        case SDL_SCANCODE_S: return 0x8;
        case SDL_SCANCODE_D: return 0x9;
        case SDL_SCANCODE_F: return 0xE;
        
        case SDL_SCANCODE_Z: return 0xA;
        case SDL_SCANCODE_X: return 0x0;
        case SDL_SCANCODE_C: return 0xB;
        case SDL_SCANCODE_V: return 0xF;
        
        default: 
            *matched = false; 
            return 0;
    }
}

/* ── input_process ────────────────────────────────────────────── */

Chip8Result input_process(Chip8 *vm, bool *quit) {
    if (!vm || !quit) return CHIP8_ERR_NULL_PTR;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = true;
        } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            /* Let Escape also close the emulator */
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                *quit = true;
                continue;
            }
            
            bool matched = false;
            uint8_t chip8_key = map_key(e.key.keysym.scancode, &matched);
            
            if (matched) {
                vm->keypad[chip8_key] = (e.type == SDL_KEYDOWN) ? 1 : 0;
            }
        }
    }
    
    return CHIP8_OK;
}

/* ── input_wait_for_key ───────────────────────────────────────── */

Chip8Result input_wait_for_key(Chip8 *vm, uint8_t *key_out, bool *quit) {
    if (!vm || !key_out || !quit) return CHIP8_ERR_NULL_PTR;

    /* 
     * Note: This is an active-wait loop function for FX0A if the CPU 
     * execution loop isn't handling it via PC rewind. Since cpu.c already 
     * implements FX0A via PC rewind, this function acts as a fallback 
     * or utility for debug pauses.
     */
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            *quit = true;
            return CHIP8_OK;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                *quit = true;
                return CHIP8_OK;
            }
            
            bool matched = false;
            uint8_t chip8_key = map_key(e.key.keysym.scancode, &matched);
            
            if (matched) {
                *key_out = chip8_key;
                vm->keypad[chip8_key] = 1;
                return CHIP8_OK;
            }
        } else if (e.type == SDL_KEYUP) {
            bool matched = false;
            uint8_t chip8_key = map_key(e.key.keysym.scancode, &matched);
            
            if (matched) {
                vm->keypad[chip8_key] = 0;
            }
        }
    }
    
    return CHIP8_OK;
}
