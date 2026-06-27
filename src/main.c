/**
 * @file main.c
 * @brief CHIP-8 Emulator main entry point and runtime loop.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "memory.h"
#include "rom.h"
#include "cpu.h"
#include "display.h"
#include "input.h"
#include "timer.h"

/* ── Audio callback ───────────────────────────────────────────── */
/*
 * Generates a basic square wave (approx 440 Hz) if the sound timer
 * is active.
 */
static void audio_callback(void *userdata, uint8_t *stream, int len) {
    Chip8 *vm = (Chip8 *)userdata;
    int16_t *buffer = (int16_t *)stream;
    int samples = len / 2;
    
    static uint32_t phase = 0;
    
    /* 440 Hz square wave. phase_inc = (freq / sample_rate) * max_uint32 */
    const uint32_t phase_inc = (uint32_t)((440.0 / 44100.0) * 4294967296.0);
    
    bool active = timer_sound_active(vm);
    
    for (int i = 0; i < samples; i++) {
        if (active) {
            /* Basic square wave: high half of phase is positive, low half negative */
            buffer[i] = (phase < 0x80000000u) ? 3000 : -3000;
        } else {
            buffer[i] = 0;
        }
        phase += phase_inc;
    }
}

/* ── main ─────────────────────────────────────────────────────── */

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ROM path>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const char *rom_path = argv[1];
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    
    Chip8 vm;
    Chip8Result res = chip8_init(&vm);
    if (res != CHIP8_OK) {
        fprintf(stderr, "chip8_init failed: %s\n", chip8_result_str(res));
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    size_t loaded = 0;
    res = rom_load(&vm, rom_path, &loaded);
    if (res != CHIP8_OK) {
        fprintf(stderr, "rom_load failed: %s\n", chip8_result_str(res));
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    printf("Loaded %zu bytes from %s\n", loaded, rom_path);
    
    DisplayCtx *display = NULL;
    res = display_init(&display, DISPLAY_DEFAULT_SCALE);
    if (res != CHIP8_OK) {
        fprintf(stderr, "display_init failed: %s\n", chip8_result_str(res));
        SDL_Quit();
        return EXIT_FAILURE;
    }
    
    /* Init Audio */
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = 44100;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audio_callback;
    want.userdata = &vm;
    
    SDL_AudioDeviceID audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_dev != 0) {
        SDL_PauseAudioDevice(audio_dev, 0); /* Unpause audio */
    } else {
        fprintf(stderr, "Warning: Failed to open audio: %s\n", SDL_GetError());
    }
    
    /* Execution Configuration */
    bool quit = false;
    const int instructions_per_frame = 12; /* Approx 720 Hz at 60 FPS */
    
    /* Main Emulator Loop */
    while (!quit) {
        uint32_t frame_start = SDL_GetTicks();
        
        /* 1. Process Input */
        res = input_process(&vm, &quit);
        if (res != CHIP8_OK) {
            fprintf(stderr, "input error: %s\n", chip8_result_str(res));
            break;
        }
        
        /* 2. Execute CPU cycles */
        for (int i = 0; i < instructions_per_frame; i++) {
            res = cpu_fetch(&vm);
            if (res != CHIP8_OK) {
                fprintf(stderr, "cpu_fetch error: %s\n", chip8_result_str(res));
                quit = true;
                break;
            }
            
            res = cpu_decode_execute(&vm);
            if (res != CHIP8_OK) {
                fprintf(stderr, "cpu_decode_execute error: %s\n", chip8_result_str(res));
                quit = true;
                break;
            }
        }
        
        /* 3. Update Timers (runs exactly once per frame, aiming for 60Hz) */
        timer_tick(&vm);
        
        /* 4. Render Display */
        res = display_render(display, &vm);
        if (res != CHIP8_OK) {
            fprintf(stderr, "display error: %s\n", chip8_result_str(res));
            break;
        }
        
        /* 5. Frame Timing (Target: ~60 FPS / 16.66 ms per frame) */
        uint32_t frame_time = SDL_GetTicks() - frame_start;
        if (frame_time < 16) {
            SDL_Delay(16 - frame_time);
        }
    }
    
    if (audio_dev != 0) {
        SDL_CloseAudioDevice(audio_dev);
    }
    
    display_destroy(display);
    SDL_Quit();
    
    return EXIT_SUCCESS;
}
